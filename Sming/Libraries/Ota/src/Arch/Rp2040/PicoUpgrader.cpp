/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RbootUpgrader.cpp
 *
 ****/

#include "include/Ota/PicoUpgrader.h"
#include <pico_partition.h>
#include <pico/bootrom.h>
#include <boot/picoboot_constants.h>
#include <pico_image.h>
#include <hardware/watchdog.h>
#include <debug_progmem.h>

using namespace Storage;

namespace
{
// In documentation but not in SDK headers
constexpr uint32_t VECTORED_BOOT_MAGIC = 0xb007c0d3;

} // namespace

namespace Ota
{
bool PicoUpgrader::begin(Partition partition, size_t size)
{
	if(size > partition.size()) {
		return false; // too big
	}

	if(!partition.isOta()) {
		return false;
	}

	// Check we're not attempting to write to the current partition
	boot_info_t boot_info{};
	int rc = rom_get_boot_info(&boot_info);
	if(rc < 0) {
		return false;
	}
	Pico::PartitionInfo info;
	Pico::getPartitionInfo(info, boot_info.partition);
	if(info.header.startOffset() == partition.address()) {
		debug_e("Write to current partition prohibited");
		return false;
	}

	this->partition = partition;
	stream =
		std::make_unique<Storage::PartitionStream>(partition, 0, size ?: partition.size(), Storage::Mode::BlockErase);

	return true;
}

size_t PicoUpgrader::write(const uint8_t* buffer, size_t size)
{
	if(!stream) {
		return 0;
	}

	if(stream->getWritePos() == 0 && size >= Pico::IMAGEDEF_END) {
		// First sector: locate IMAGE_DEF word
		auto info = Pico::findImageDef(const_cast<uint8_t*>(buffer + Pico::IMAGEDEF_START));

		// Set TBYB bit in type flags so image doesn't get picked up automatically
		// This may get reset in `setBootPartition`
		auto flags = info.getFlags();
		info.setFlags(flags | PICOBIN_IMAGE_TYPE_EXE_TBYB_BITS);
	}

	return stream->write(buffer, size);
}

bool PicoUpgrader::end()
{
	if(!stream) {
		return false;
	}

	stream.reset();
	return true;
}

bool PicoUpgrader::setBootPartition(Partition partition, bool save)
{
	if(partition.type() != Storage::Partition::Type::app) {
		debug_e("setBootPartition requires application partition");
		return false;
	}

	// Read IMAGE_DEF from partition for inspection and update (if required)

	uint8_t buffer[INTERNAL_FLASH_SECTOR_SIZE];
	auto ok = partition.read(0, buffer, sizeof(buffer));
	debug_i("partition.read(%p, %u) %u", partition.address(), sizeof(buffer), ok);
	auto info = Pico::findImageDef(buffer + Pico::IMAGEDEF_START);
	if(!info) {
		debug_e("No IMAGE_DEF found in %s", partition.name().c_str());
		return false;
	}

	debug_i("Image flags 0x%04x, version %u.%u", info.getFlags(), info.getVersionMajor(), info.getVersionMinor());

	uint32_t rebootType;
	uint32_t p0;

	if(save) {
		// To ensure ROM code selects this partition for boot, image must have the highest version number

		// Determine highest version number for all other application images
		// Note: This only applies to A/B pairs, so we might revise this to simply consider slot A vs. slot B
		// TODO: For maximum compatibility search pico partitions directly in case they've been customised
		uint32_t maxVersion = 0;
		for(auto part : spiFlash->partitions().find(Storage::Partition::Type::app)) {
			if(part.address() == partition.address()) {
				continue;
			}
			uint8_t hdr[Pico::IMAGEDEF_BUFFER_SIZE];
			part.read(Pico::IMAGEDEF_START, hdr, sizeof(hdr));
			auto hdrInfo = Pico::findImageDef(hdr);
			debug_i("%s version %u.%u", part.name().c_str(), hdrInfo.getVersionMajor(), hdrInfo.getVersionMinor());
			maxVersion = std::max(maxVersion, hdrInfo.getVersion());
		}

		bool imageDefChanged = false;

		// Check if we need to increase the existing version number
		auto version = info.getVersion();
		if(version <= maxVersion) {
			auto newVersion = maxVersion + 1;
			debug_i("Changing image version from %04x -> %04x", version, newVersion);
			info.setVersion(newVersion);
			imageDefChanged = true;
		}

		// Clear TBYB bit if required
		auto flags = info.getFlags();
		if(flags & PICOBIN_IMAGE_TYPE_EXE_TBYB_BITS) {
			debug_i("Clearing TBYB flag");
			info.setFlags(flags & ~PICOBIN_IMAGE_TYPE_EXE_TBYB_BITS);
			imageDefChanged = true;
		}

		// Write out any changes to partition
		if(imageDefChanged) {
			partition.erase_range(0, INTERNAL_FLASH_SECTOR_SIZE);
			partition.write(0, buffer, sizeof(buffer));
		}

		rebootType = REBOOT2_FLAG_REBOOT_TYPE_NORMAL;
		p0 = 0;
	} else {
		// For temporary boot the TBYB bit must be set
		auto flags = info.getFlags();
		if(!(flags & PICOBIN_IMAGE_TYPE_EXE_TBYB_BITS)) {
			info.setFlags(flags | PICOBIN_IMAGE_TYPE_EXE_TBYB_BITS);
			debug_i("Set flags 0x%04x", info.getFlags());
			partition.erase_range(0, INTERNAL_FLASH_SECTOR_SIZE);
			partition.write(0, buffer, sizeof(buffer));
		}

		rebootType = REBOOT2_FLAG_REBOOT_TYPE_FLASH_UPDATE;
		p0 = XIP_BASE + partition.address();
	}

	// This won't actually reboot until application calls `system_restart`
	// since Sming resets watchdog in main loop
	int rc = rom_reboot(rebootType, 5000, p0, 0);
	return rc >= 0;
}

Partition PicoUpgrader::getBootPartition()
{
	// Scratch registers may indicate a vectored boot
	uint32_t addr = watchdog_hw->scratch[7];
	if(isFlashPtr(addr) && watchdog_hw->scratch[4] == VECTORED_BOOT_MAGIC &&
	   watchdog_hw->scratch[5] ^ VECTORED_BOOT_MAGIC == addr) {
		// Valid temporary boot
		return spiFlash->partitions().find(addr - XIP_BASE);
	}

	// Find application image with highest version
	Partition bootPart;
	uint32_t maxVersion = 0;
	for(auto part : spiFlash->partitions().find(Storage::Partition::Type::app)) {
		uint8_t hdr[Pico::IMAGEDEF_BUFFER_SIZE];
		part.read(Pico::IMAGEDEF_START, hdr, sizeof(hdr));
		auto info = Pico::findImageDef(hdr);
		debug_i("%s version %u.%u", part.name().c_str(), info.getVersionMajor(), info.getVersionMinor());
		// Ignore TBYB images
		if(info.getFlags() & PICOBIN_IMAGE_TYPE_EXE_TBYB_BITS) {
			continue;
		}
		auto version = info.getVersion();
		if(version <= maxVersion) {
			continue;
		}
		maxVersion = version;
		bootPart = part;
	}
	return bootPart;
}

Partition PicoUpgrader::getRunningPartition()
{
	boot_info_t boot_info{};
	int rc = rom_get_boot_info(&boot_info);
	if(rc < 0) {
		return {};
	}
	Pico::PartitionInfo info;
	Pico::getPartitionInfo(info, boot_info.partition);
	uint32_t addr = info.header.startOffset();
	return spiFlash->partitions().find(addr);
}

Partition PicoUpgrader::getNextBootPartition(Partition startFrom)
{
	if(!startFrom) {
		startFrom = getRunningPartition();
	}
	Partition first;
	bool useNext{false};
	for(auto part : spiFlash->partitions()) {
		if(!part.isOta()) {
			continue;
		}
		if(useNext) {
			return part;
		}
		if(part.address() == startFrom.address()) {
			useNext = true;
		} else if(!first) {
			first = part;
		}
	}

	if(useNext) {
		return first;
	}

	return {};
}

} // namespace Ota
