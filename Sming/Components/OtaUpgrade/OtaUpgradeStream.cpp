#include "OtaUpgradeStream.h"
#include <algorithm>
#include <esp_spi_flash.h>

// symbol provided by appcode (generated source file)
#ifdef OTA_SIGNED
extern "C" uint8_t OTAUpgrade_PublicKey_P[];
#else
extern "C" void MD5Init(void *ctx);
extern "C" void MD5Update(void *ctx, const void *buf, uint32_t len);
extern "C" void MD5Final(uint8_t digest[16], void *ctx);
#endif

#ifdef OTA_DOWNGRADE_PROTECTION
extern "C" const uint64_t OTA_BuildTimestamp PROGMEM;
#endif

OtaUpgradeStream::Slot::Slot()
{
	// Get parameters of the slot where the firmware image should be stored.
	const rboot_config bootConfig = rboot_get_config();
	uint8_t currentSlot = bootConfig.current_rom;
	index = (currentSlot == 0 ? 1 : 0);
	address = bootConfig.roms[index];
	size = 0x100000 - (address & 0xFFFFF);

	const auto limitSize = [&](uint32_t otherAddress) {
		if (otherAddress > address) {
			size = std::min(size, otherAddress - address);
		}
	};

	limitSize(flashmem_get_size_bytes());
	for (uint8_t i = 0; i < bootConfig.count; ++i) {
		if (i != currentSlot) {
			limitSize(bootConfig.roms[i]);
		}
	}
#ifdef RBOOT_SPIFFS_0
	limitSize(RBOOT_SPIFFS_0);
#endif
#ifdef RBOOT_SPIFFS_1
	limitSize(RBOOT_SPIFFS_0);
#endif
}

OtaUpgradeStream::OtaUpgradeStream()
{
#ifdef OTA_SIGNED
	crypto_sign_init(&verifierState);
#else
	MD5Init(&md5Context);
#endif
	setupChunk(StateHeader, fileHeader);
}

bool OtaUpgradeStream::consume(const uint8_t *&data, size_t &size)
{
	size_t chunkSize = std::min(size, remainingBytes);
	if (state != StateRomSignature) {
#ifdef OTA_SIGNED
		crypto_sign_update(&verifierState, static_cast<const unsigned char *>(data), chunkSize);
#else
		MD5Update(&md5Context, data, chunkSize);
#endif
	}

	if (destinationPtr != nullptr) {
		memcpy(destinationPtr, data, chunkSize);
		destinationPtr += chunkSize;
	}
	remainingBytes -= chunkSize;
	data += chunkSize;
	size -= chunkSize;
	if (remainingBytes == 0) {
		destinationPtr = nullptr;
		return true;
	} else {
		return false;
	}
}

void OtaUpgradeStream::setError(ErrorCode code) 
{
	assert(code != NoError);
	debug_e("Error: %s\n", errorToString(code).c_str());	
	errorCode = code;
	state = StateError;
}

void OtaUpgradeStream::nextRom() 
{
	if (romIndex < fileHeader.romCount) {
		++romIndex;
		setupChunk(StateRomHeader, romHeader);
	} else {
		setupChunk(StateRomSignature, signature);
	}
}

void OtaUpgradeStream::processRomHeader() 
{
	bool addressMatch = (slot.address & 0xFFFFF) == (romHeader.address & 0xFFFFF);
	if (!slot.updated && addressMatch) {
		if (romHeader.size <= slot.size) {
			debug_i("Update slot %u [0x%08X..0x%08X)\n", slot.index, slot.address, slot.address + romHeader.size);
			rbootWriteStatus = rboot_write_init(slot.address);
			setupChunk(StateWriteRom, romHeader.size);
		} else {
			setError(RomTooLargeError);
		}
		return;
	}
	
	debug_i("Skip ROM image for [0x%08X..0x%08X)\n", romHeader.address, romHeader.address + romHeader.size);
	setupChunk(StateSkipRom, romHeader.size);
}

void OtaUpgradeStream::verifyRoms()
{
	state = StateRomsComplete;

	debug_d("Signature/Checksum: ");
	for (auto b: signature) debug_d("%02X", b);
	debug_d("\n");

#ifdef OTA_SIGNED
	uint8_t verificationKey[crypto_sign_PUBLICKEYBYTES];
	memcpy_P(verificationKey, OTAUpgrade_PublicKey_P, sizeof(verificationKey));
	const bool signatureMismatch = (crypto_sign_final_verify(&verifierState, signature, verificationKey) != 0);
#else
	uint8_t expectedChecksum[sizeof(signature)];
	MD5Final(expectedChecksum, &md5Context);
	const bool signatureMismatch = (memcmp(expectedChecksum, signature, sizeof(signature)) != 0);
#endif
	
	if (signatureMismatch) {
		if (slot.updated) {
			// Destroy start sector of updated ROM to avoid accidentally booting an unsanctioned firmware
			flashmem_erase_sector(slot.address / SECTOR_SIZE);
		}
		setError(VerificationError);
		return;
	}

	// In a future extension, there might be OTA files without ROM images (SPIFFS-only update, etc.)
	if (fileHeader.romCount == 0) {
		return;
	}

	debug_i("ROM update complete\n");
	if (!slot.updated) {
		setError(NoRomFoundError);
		return;
	}

	if (rboot_set_current_rom(slot.index)) {
		debug_i("ROM %u activated\n", slot.index);
	} else {
		setError(RomActivationError);
	}
}

size_t OtaUpgradeStream::write(const uint8_t* data, size_t size) 
{
	size_t available = size;

	while(!hasError() && (available > 0)) {
		switch(state) {
		case StateHeader:
			if (consume(data, available)) {
				if (fileHeader.magic == HEADER_MAGIC_EXPECTED) {
#ifdef OTA_DOWNGRADE_PROTECTION
					uint64_t buildTimestampFirmware;
					memcpy_P(&buildTimestampFirmware, &OTA_BuildTimestamp, sizeof(OTA_BuildTimestamp));
					debug_i("Build timestamp of current firmware: %ull\n", buildTimestamp);
					uint64_t buildTimestampUpgrade = ((uint64_t)fileHeader.buildTimestampHigh << 32) | fileHeader.buildTimestampLow;
					debug_i("Build timestamp of OTA upgrade file: %ull\n", buildTimestampUpgrade);
					if (buildTimestampUpgrade < buildTimestampFirmware) {
						setError(DowngradeError);
						break;
					}
#endif
					debug_i("Starting firmware upgrade, receive %u image(s)\n", fileHeader.romCount);
					nextRom();
				} else {
					setError(InvalidFormatError);
				}
			}
			break;

		case StateRomHeader:
			if (consume(data, available)) {
				processRomHeader();
			}
			break;

		case StateWriteRom:
			{
				bool ok = rboot_write_flash(&rbootWriteStatus, const_cast<uint8_t *>(data), std::min(remainingBytes, available));
				if (ok) {
					if (consume(data, available)) {
						ok = slot.updated = rboot_write_end(&rbootWriteStatus);
						nextRom();
					}
				}
				if (!ok) {
					setError(FlashWriteError);
				}
			}
			break;

		case StateSkipRom:
			if (consume(data, available)) {
				nextRom();
			}
			break;

		case StateRomSignature:
			if (consume(data, available)) {
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
	
	return size - available;
}

String OtaUpgradeStream::errorToString(ErrorCode code)
{
	switch(code) {
	case NoError: 
		return nullptr;
	case InvalidFormatError: 
		return F("Invalid/Unrecognized upgrade image format");
	case UnsupportedDataError: 
		return F("OTA image contains unsupported extended data.");
	case NoRomFoundError:
		return F("No suitable ROM image found");
	case RomTooLargeError:
		return F("ROM image too large");
	case DowngradeError:
		return F("Downgrade not allowed");
	case VerificationError:
#ifdef OTA_SIGNED
		return F("Signature verification failed");
#else
		return F("Checksum mismatch");
#endif
	case FlashWriteError:
		return F("Error while writing Flash memory");
	case RomActivationError:
		return F("Could not activate updated ROM");
	case InternalError:
		return F("Internal error");
	default:
		return F("<unknown error>");
	}
}
