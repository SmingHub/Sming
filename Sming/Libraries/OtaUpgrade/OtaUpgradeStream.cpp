/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OtaUpgradeStream.cpp
 *
 ****/

#include "OtaUpgradeStream.h"
#include <algorithm>
#include <esp_spi_flash.h>
#include <FlashString/Array.hpp>
#include <Data/HexString.h>

// Keys defined by `appcode/keys.cpp` which is compiled with application
#ifdef ENABLE_OTA_SIGNING
extern "C" const FSTR::Array<uint8_t> OTAUpgrade_SignatureVerificationKey;
#endif

#ifdef ENABLE_OTA_ENCRYPTION
#include <sodium/utils.h>
extern "C" const FSTR::Array<uint8_t> OTAUpgrade_DecryptionKey;
#endif

#ifndef ENABLE_OTA_DOWNGRADE
extern "C" const uint64_t OTA_BuildTimestamp PROGMEM;
#endif

#ifdef ENABLE_OTA_SIGNING
static const uint32_t HEADER_MAGIC_EXPECTED = 0xf01af02a;
#else
static const uint32_t HEADER_MAGIC_EXPECTED = 0xf01af020;
#endif

DECLARE_FSTR_ARRAY(OTAUpgrade_AppFlashRegionOffsets, uint32_t)

OtaUpgradeStream::Slot::Slot()
{
	// Get parameters of the slot where the firmware image should be stored.
	const rboot_config bootConfig = rboot_get_config();
	uint8_t currentSlot = bootConfig.current_rom;
	index = (currentSlot == 0 ? 1 : 0);
	address = bootConfig.roms[index];
	size = 0x100000 - (address & 0xFFFFF);

	const auto limitSize = [&](uint32_t otherAddress) {
		if(otherAddress > address) {
			size = std::min(size, otherAddress - address);
		}
	};

	limitSize(flashmem_get_size_bytes());
	for(uint8_t i = 0; i < bootConfig.count; ++i) {
		if(i != currentSlot) {
			limitSize(bootConfig.roms[i]);
		}
	}

	for(auto offset : OTAUpgrade_AppFlashRegionOffsets) {
		limitSize(offset);
	}
}

OtaUpgradeStream::OtaUpgradeStream()
{
#ifdef ENABLE_OTA_ENCRYPTION
	encryption.fragmentPtr = encryption.header;
	encryption.remainingBytes = sizeof(encryption.header);
#endif

#ifdef ENABLE_OTA_SIGNING
	crypto_sign_init(&verifierState);
#else
	MD5Init(&md5Context);
#endif

	setupChunk(StateHeader, fileHeader);
}

OtaUpgradeStream::~OtaUpgradeStream()
{
#ifdef ENABLE_OTA_ENCRYPTION
	free(encryption.buffer);
#endif
}

bool OtaUpgradeStream::consume(const uint8_t*& data, size_t& size)
{
	size_t chunkSize = std::min(size, remainingBytes);
	if(state != StateRomSignature) {
#ifdef ENABLE_OTA_SIGNING
		crypto_sign_update(&verifierState, static_cast<const unsigned char*>(data), chunkSize);
#else
		MD5Update(&md5Context, data, chunkSize);
#endif
	}

	if(destinationPtr != nullptr) {
		memcpy(destinationPtr, data, chunkSize);
		destinationPtr += chunkSize;
	}
	remainingBytes -= chunkSize;
	data += chunkSize;
	size -= chunkSize;
	if(remainingBytes == 0) {
		destinationPtr = nullptr;
		return true;
	} else {
		return false;
	}
}

void OtaUpgradeStream::setError(ErrorCode code)
{
	assert(code != NoError);
	debug_e("Error: %s", errorToString(code).c_str());
	errorCode = code;
	state = StateError;
}

void OtaUpgradeStream::nextRom()
{
	if(romIndex < fileHeader.romCount) {
		++romIndex;
		setupChunk(StateRomHeader, romHeader);
	} else {
		setupChunk(StateRomSignature, signature);
	}
}

void OtaUpgradeStream::processRomHeader()
{
	bool addressMatch = (slot.address & 0xFFFFF) == (romHeader.address & 0xFFFFF);
	if(!slot.updated && addressMatch) {
		if(romHeader.size <= slot.size) {
			debug_i("Update slot %u [0x%08X..0x%08X)", slot.index, slot.address, slot.address + romHeader.size);
			rbootWriteStatus = rboot_write_init(slot.address);
			setupChunk(StateWriteRom, romHeader.size);
		} else {
			setError(RomTooLargeError);
		}
		return;
	}

	debug_i("Skip ROM image for [0x%08X..0x%08X)", romHeader.address, romHeader.address + romHeader.size);
	setupChunk(StateSkipRom, romHeader.size);
}

void OtaUpgradeStream::verifyRoms()
{
	state = StateRomsComplete;

	debug_d("Signature/Checksum: %s", makeHexString(signature, sizeof(signature), ' ').c_str());

#ifdef ENABLE_OTA_SIGNING
	assert(OTAUpgrade_SignatureVerificationKey.length() == crypto_sign_PUBLICKEYBYTES);
	LOAD_FSTR_ARRAY(verificationKey, OTAUpgrade_SignatureVerificationKey);
	const bool signatureMismatch = (crypto_sign_final_verify(&verifierState, signature, verificationKey) != 0);
#else
	uint8_t expectedChecksum[MD5_SIZE];
	MD5Final(expectedChecksum, &md5Context);
	const bool signatureMismatch = (memcmp(expectedChecksum, signature, sizeof(signature)) != 0);
#endif

	if(signatureMismatch) {
		if(slot.updated) {
			// Destroy start sector of updated ROM to avoid accidentally booting an unsanctioned firmware
			flashmem_erase_sector(slot.address / SECTOR_SIZE);
		}
		setError(VerificationError);
		return;
	}

	// In a future extension, there might be OTA files without ROM images (SPIFFS-only update, etc.)
	if(fileHeader.romCount == 0) {
		return;
	}

	debug_i("ROM update complete");
	if(!slot.updated) {
		setError(NoRomFoundError);
		return;
	}

	if(rboot_set_current_rom(slot.index)) {
		debug_i("ROM %u activated", slot.index);
	} else {
		setError(RomActivationError);
	}
}

size_t OtaUpgradeStream::write(const uint8_t* data, size_t size)
{
#ifndef ENABLE_OTA_ENCRYPTION
	return process(data, size);
#else
	size_t availableBytes = size;

	while(!hasError() && (availableBytes > 0)) {
		std::size_t toConsume = std::min(encryption.remainingBytes, availableBytes);
		memcpy(encryption.fragmentPtr, data, toConsume);
		availableBytes -= toConsume;
		data += toConsume;
		encryption.fragmentPtr += toConsume;
		encryption.remainingBytes -= toConsume;

		if(encryption.remainingBytes == 0) {
			switch(encryption.fragment) {
			case encryption.FragmentHeader: {
				assert(OTAUpgrade_DecryptionKey.length() == crypto_secretstream_xchacha20poly1305_KEYBYTES);
				LOAD_FSTR_ARRAY(key, OTAUpgrade_DecryptionKey);
				bool ok =
					(crypto_secretstream_xchacha20poly1305_init_pull(&encryption.state, encryption.header, key) == 0);
				sodium_memzero(key, sizeof(key));
				if(!ok) {
					setError(DecryptionError);
					break;
				}
				encryption.fragment = encryption.FragmentChunkSize;
				encryption.fragmentPtr = reinterpret_cast<uint8_t*>(&encryption.chunkSizeMinusOne);
				encryption.remainingBytes = sizeof(encryption.chunkSizeMinusOne);
			} break;

			case encryption.FragmentChunkSize:
				encryption.remainingBytes = 1 + encryption.chunkSizeMinusOne;
				if(encryption.buffer == nullptr || encryption.bufferSize < encryption.remainingBytes) {
					free(encryption.buffer);
					encryption.buffer = (uint8_t*)malloc(encryption.remainingBytes);
					if(encryption.buffer == nullptr) {
						setError(OutOfMemoryError);
						break;
					}
					encryption.bufferSize = encryption.remainingBytes;
				}
				encryption.fragmentPtr = encryption.buffer;
				encryption.fragment = encryption.FragmentChunk;
				break;

			case encryption.FragmentChunk: {
				unsigned char tag;
				size_t chiperTextLength = 1 + encryption.chunkSizeMinusOne;
				unsigned long long messageLength = 0;
				bool ok = (crypto_secretstream_xchacha20poly1305_pull(&encryption.state, encryption.buffer,
																	  &messageLength, &tag, encryption.buffer,
																	  chiperTextLength, NULL, 0) == 0);
				if(!ok || messageLength > encryption.bufferSize) {
					setError(DecryptionError);
					break;
				}
				if(tag != crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
					encryption.fragment = encryption.FragmentChunkSize;
					encryption.fragmentPtr = reinterpret_cast<uint8_t*>(&encryption.chunkSizeMinusOne);
					encryption.remainingBytes = sizeof(encryption.chunkSizeMinusOne);
				} else {
					encryption.fragment = encryption.FragmentNone;
				}

				process(encryption.buffer, static_cast<size_t>(messageLength));
			} break;

			case encryption.FragmentNone:
				setError(InvalidFormatError);
				break;

			default:
				setError(InternalError);
			}
		}
	}
	return size - availableBytes;
#endif
}

size_t OtaUpgradeStream::process(const uint8_t* data, size_t size)
{
	size_t availableBytes = size;

	while(!hasError() && (availableBytes > 0)) {
		switch(state) {
		case StateHeader:
			if(consume(data, availableBytes)) {
				if(fileHeader.magic == expectedHeaderMagic) {
#ifndef ENABLE_OTA_DOWNGRADE
					uint64_t buildTimestampFirmware;
					memcpy_P(&buildTimestampFirmware, &OTA_BuildTimestamp, sizeof(OTA_BuildTimestamp));
					debug_i("Build timestamp of current firmware: %ull", buildTimestampFirmware);
					uint64_t buildTimestampUpgrade =
						((uint64_t)fileHeader.buildTimestampHigh << 32) | fileHeader.buildTimestampLow;
					debug_i("Build timestamp of OTA upgrade file: %ull", buildTimestampUpgrade);
					if(buildTimestampUpgrade < buildTimestampFirmware) {
						setError(DowngradeError);
						break;
					}
#endif
					debug_i("Starting firmware upgrade, receive %u image(s)", fileHeader.romCount);
					nextRom();
				} else {
					setError(InvalidFormatError);
				}
			}
			break;

		case StateRomHeader:
			if(consume(data, availableBytes)) {
				processRomHeader();
			}
			break;

		case StateWriteRom: {
			bool ok = rboot_write_flash(&rbootWriteStatus, const_cast<uint8_t*>(data),
										std::min(remainingBytes, availableBytes));
			if(ok) {
				if(consume(data, availableBytes)) {
					ok = slot.updated = rboot_write_end(&rbootWriteStatus);
					nextRom();
				}
			}
			if(!ok) {
				setError(FlashWriteError);
			}
		} break;

		case StateSkipRom:
			if(consume(data, availableBytes)) {
				nextRom();
			}
			break;

		case StateRomSignature:
			if(consume(data, availableBytes)) {
				verifyRoms();
			}
			break;

		case StateRomsComplete:
			setError(UnsupportedDataError);
			break;

		case StateError:
			break;
		default:
			setError(InternalError);
			break;
		}
	}

	return size - availableBytes;
}

String OtaUpgradeStream::errorToString(ErrorCode code)
{
	if(code == NoError) {
		return nullptr;
	}

	switch(code) {
#define XX(tag, text, desc)                                                                                            \
	case tag##Error:                                                                                                   \
		return F(text);
		OTA_ERRORCODE_MAP(XX)
#undef XX
	default:
		return F("<unknown error>");
	}
}
