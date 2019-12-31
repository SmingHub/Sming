/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BasicOtaUpgradeStream.cpp
 *
 ****/

#include "OtaUpgradeStream.h"
#include <algorithm>
#include <esp_spi_flash.h>
#include <Data/HexString.h>
#include <FlashString/Array.hpp>

#ifndef ENABLE_OTA_DOWNGRADE
extern "C" const uint64_t OTA_BuildTimestamp PROGMEM;
#endif

DECLARE_FSTR_ARRAY(OTAUpgrade_AppFlashRegionOffsets, uint32_t);

BasicOtaUpgradeStream::Slot::Slot()
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

BasicOtaUpgradeStream::BasicOtaUpgradeStream()
{
	setupChunk(StateHeader, fileHeader);
}

bool BasicOtaUpgradeStream::consume(const uint8_t*& data, size_t& size)
{
	size_t chunkSize = std::min(size, remainingBytes);
	if(state != StateVerifyRoms) {
		verifier.update(data, chunkSize);
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

void BasicOtaUpgradeStream::setError(ErrorCode code)
{
	assert(code != NoError);
	debug_e("Error: %s", errorToString(code).c_str());
	errorCode = code;
	state = StateError;
}

void BasicOtaUpgradeStream::nextRom()
{
	if(romIndex < fileHeader.romCount) {
		++romIndex;
		setupChunk(StateRomHeader, romHeader);
	} else {
		setupChunk(StateVerifyRoms, verificationData);
	}
}

void BasicOtaUpgradeStream::processRomHeader()
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

void BasicOtaUpgradeStream::verifyRoms()
{
	state = StateRomsComplete;

	debug_d("Signature/Checksum: %s", makeHexString(verificationData, sizeof(verificationData), ' ').c_str());

	if(!verifier.verify(verificationData)) {
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

size_t BasicOtaUpgradeStream::write(const uint8_t* data, size_t size)
{
	const size_t origSize = size;

	while(!hasError() && (size > 0)) {
		switch(state) {
		case StateHeader:
			if(consume(data, size)) {
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
			if(consume(data, size)) {
				processRomHeader();
			}
			break;

		case StateWriteRom: {
			bool ok = rboot_write_flash(&rbootWriteStatus, const_cast<uint8_t*>(data), std::min(remainingBytes, size));
			if(ok) {
				if(consume(data, size)) {
					ok = slot.updated = rboot_write_end(&rbootWriteStatus);
					nextRom();
				}
			}
			if(!ok) {
				setError(FlashWriteError);
			}
		} break;

		case StateSkipRom:
			if(consume(data, size)) {
				nextRom();
			}
			break;

		case StateVerifyRoms:
			if(consume(data, size)) {
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

	return origSize - size;
}

String BasicOtaUpgradeStream::errorToString(ErrorCode code)
{
	switch(code) {
	case NoError:
		return nullptr;
	case InvalidFormatError:
		return F("Invalid/Unrecognized upgrade image format");
	case UnsupportedDataError:
		return F("Upgrade image contains unsupported extended data.");
	case DecryptionError:
		return F("Upgrade image decryption failed.");
	case NoRomFoundError:
		return F("No suitable ROM image found");
	case RomTooLargeError:
		return F("ROM image too large");
	case DowngradeError:
		return F("Downgrade not allowed");
	case VerificationError:
		return F("Signature/Checksum verification failed");
	case FlashWriteError:
		return F("Error while writing Flash memory");
	case RomActivationError:
		return F("Could not activate updated ROM");
	case OutOfMemoryError:
		return F("Out of memory. Allocation failed.");
	case InternalError:
		return F("Internal error");
	default:
		return F("<unknown error>");
	}
}
