/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BasicStream.cpp
 *
 ****/

#include "BasicStream.h"
#include <algorithm>
#include <esp_spi_flash.h>
#include <Data/HexString.h>
#include <FlashString/Array.hpp>

extern "C" uint32 user_rf_cal_sector_set(void);

namespace OtaUpgrade
{
#ifndef ENABLE_OTA_DOWNGRADE
extern const uint64_t BuildTimestamp;
#endif

DECLARE_FSTR_ARRAY(AppFlashRegionOffsets, uint32_t);

BasicStream::Slot::Slot()
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
	limitSize(user_rf_cal_sector_set() * INTERNAL_FLASH_SECTOR_SIZE);
	for(uint8_t i = 0; i < bootConfig.count; ++i) {
		if(i != currentSlot) {
			limitSize(bootConfig.roms[i]);
		}
	}

	for(auto offset : AppFlashRegionOffsets) {
		limitSize(offset);
	}
}

BasicStream::BasicStream()
{
	setupChunk(State::Header, fileHeader);
	romHeader = {}; // make cppcheck happy (will be overwritten with content from received upgrade image)
}

bool BasicStream::consume(const uint8_t*& data, size_t& size)
{
	size_t chunkSize = std::min(size, remainingBytes);
	if(state != State::VerifyRoms) {
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

void BasicStream::setError(Error code)
{
	assert(code != Error::None);
	debug_e("Error: %s", errorToString(code).c_str());
	errorCode = code;
	state = State::Error;
}

void BasicStream::nextRom()
{
	if(romIndex < fileHeader.romCount) {
		++romIndex;
		setupChunk(State::RomHeader, romHeader);
	} else {
		setupChunk(State::VerifyRoms, verificationData);
	}
}

void BasicStream::processRomHeader()
{
	bool addressMatch = (slot.address & 0xFFFFF) == (romHeader.address & 0xFFFFF);
	if(!slot.updated && addressMatch) {
		if(romHeader.size <= slot.size) {
			debug_i("Update slot %u [0x%08X..0x%08X)", slot.index, slot.address, slot.address + romHeader.size);
			rbootWriteStatus = rboot_write_init(slot.address);
			setupChunk(State::WriteRom, romHeader.size);
		} else {
			setError(Error::RomTooLarge);
		}
		return;
	}

	debug_i("Skip ROM image for [0x%08X..0x%08X)", romHeader.address, romHeader.address + romHeader.size);
	setupChunk(State::SkipRom, romHeader.size);
}

void BasicStream::verifyRoms()
{
	state = State::RomsComplete;

	debug_d("Signature/Checksum: %s", makeHexString(verificationData.data(), verificationData.size(), ' ').c_str());

	if(!verifier.verify(verificationData)) {
		if(slot.updated) {
			// Destroy start sector of updated ROM to avoid accidentally booting an unsanctioned firmware
			flashmem_erase_sector(slot.address / SECTOR_SIZE);
		}
		setError(Error::VerificationFailed);
		return;
	}

	// In a future extension, there might be OTA files without ROM images (SPIFFS-only update, etc.)
	if(fileHeader.romCount == 0) {
		return;
	}

	debug_i("ROM update complete");
	if(!slot.updated) {
		setError(Error::NoRomFound);
		return;
	}

	if(rboot_set_current_rom(slot.index)) {
		debug_i("ROM %u activated", slot.index);
	} else {
		setError(Error::RomActivationFailed);
	}
}

size_t BasicStream::write(const uint8_t* data, size_t size)
{
	const size_t origSize = size;

	while(!hasError() && (size > 0)) {
		switch(state) {
		case State::Header:
			if(consume(data, size)) {
				if(fileHeader.magic == expectedHeaderMagic) {
#ifndef ENABLE_OTA_DOWNGRADE
					const auto buildTimestampFirmware = FSTR::readValue(&BuildTimestamp);
					debug_i("Build timestamp of current firmware: %ull", buildTimestampFirmware);
					uint64_t buildTimestampUpgrade =
						((uint64_t)fileHeader.buildTimestampHigh << 32) | fileHeader.buildTimestampLow;
					debug_i("Build timestamp of OTA upgrade file: %ull", buildTimestampUpgrade);
					if(buildTimestampUpgrade < buildTimestampFirmware) {
						setError(Error::DowngradeNotAllowed);
						break;
					}
#endif
					debug_i("Starting firmware upgrade, receive %u image(s)", fileHeader.romCount);
					nextRom();
				} else {
					setError(Error::InvalidFormat);
				}
			}
			break;

		case State::RomHeader:
			if(consume(data, size)) {
				processRomHeader();
			}
			break;

		case State::WriteRom: {
			bool ok = rboot_write_flash(&rbootWriteStatus, const_cast<uint8_t*>(data), std::min(remainingBytes, size));
			if(ok) {
				if(consume(data, size)) {
					ok = slot.updated = rboot_write_end(&rbootWriteStatus);
					nextRom();
				}
			}
			if(!ok) {
				setError(Error::FlashWriteFailed);
			}
		} break;

		case State::SkipRom:
			if(consume(data, size)) {
				nextRom();
			}
			break;

		case State::VerifyRoms:
			if(consume(data, size)) {
				verifyRoms();
			}
			break;

		case State::RomsComplete:
			setError(Error::UnsupportedData);
			break;

		case State::Error:
			break;
		default:
			setError(Error::Internal);
			break;
		}
	}

	return origSize - size;
}

String BasicStream::errorToString(Error code)
{
	switch(code) {
	case Error::None:
		return nullptr;
	case Error::InvalidFormat:
		return F("Invalid/Unrecognized upgrade image format");
	case Error::UnsupportedData:
		return F("Upgrade image contains unsupported extended data.");
	case Error::DecryptionFailed:
		return F("Upgrade image decryption failed.");
	case Error::NoRomFound:
		return F("No suitable ROM image found");
	case Error::RomTooLarge:
		return F("ROM image too large");
	case Error::DowngradeNotAllowed:
		return F("Downgrade not allowed");
	case Error::VerificationFailed:
		return F("Signature/Checksum verification failed");
	case Error::FlashWriteFailed:
		return F("Error while writing Flash memory");
	case Error::RomActivationFailed:
		return F("Could not activate updated ROM");
	case Error::OutOfMemory:
		return F("Out of memory. Allocation failed.");
	case Error::Internal:
		return F("Internal error");
	default:
		return F("<unknown error>");
	}
}

} // namespace OtaUpgrade
