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
#include <Storage/SpiFlash.h>
#include <pico_partition.h>
#include <pico/bootrom.h>
#include <boot/picoboot_constants.h>
#include <boot/picobin.h>
#include <hardware/watchdog.h>
#include <debug_progmem.h>

using namespace Storage;

namespace Ota
{
// In documentation but not in SDK headers
constexpr uint32_t VECTORED_BOOT_MAGIC = 0xb007c0d3;

// Range where IMAGE_DEF is expected at start of partition (in words)
constexpr uint32_t HEADER_START_OFFSET = 0x110 / sizeof(uint32_t);
constexpr uint32_t HEADER_END_OFFSET = 0x200 / sizeof(uint32_t);

using SectorBuffer = uint32_t[INTERNAL_FLASH_SECTOR_SIZE / sizeof(uint32_t)];
using HeaderBuffer = uint32_t[HEADER_END_OFFSET - HEADER_START_OFFSET];

struct ImageDefInfo {
	uint32_t* imageType;
	uint32_t* version;

	explicit operator bool() const
	{
		return imageType && version;
	}

	uint16_t getFlags() const
	{
		return imageType ? (imageType[0] >> 16) : 0;
	}

	void setFlags(uint16_t flags)
	{
		if(imageType) {
			*imageType = (*imageType & 0xffff) | (flags << 16);
		}
	}

	uint32_t getVersion() const
	{
		return version ? version[1] : 0;
	}

	void setVersion(uint32_t value) const
	{
		if(version) {
			version[1] = value;
		}
	}

	uint16_t getVersionMajor() const
	{
		return getVersion() >> 16;
	}

	uint16_t getVersionMinor() const
	{
		return getVersion() & 0xffff;
	}
};

/*
	Expected IMAGE_DEF block:

	d3 de ff ff		PICOBIN_BLOCK_MARKER_START
	42 01 21 10		PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE
						Flags: 0x1021
	48 02 00 00		PICOBIN_BLOCK_ITEM_1BS_VERSION
	06 00 01 00			Major 0x0001, Minor 0x0006
	ff 03 00 00		PICOBIN_BLOCK_ITEM_2BS_LAST
	a4 86 00 00			relative pointer to next block loop
	79 35 12 ab			PICOBIN_BLOCK_MARKER_END
*/
ImageDefInfo findImageDef(uint32_t* buffer)
{
	m_printHex("BUF", buffer, HEADER_END_OFFSET - HEADER_START_OFFSET);
	unsigned offset = 0;
	const unsigned maxOffset = HEADER_END_OFFSET - HEADER_START_OFFSET;
	while(buffer[offset++] != PICOBIN_BLOCK_MARKER_START) {
		if(offset >= maxOffset) {
			return {};
		}
	}

	ImageDefInfo info{};
	while(offset < maxOffset) {
		uint32_t w = buffer[offset];
		uint16_t len;
		if(w & 0x80) {
			len = (w >> 8) & 0xffff;
		} else {
			len = (w >> 8) & 0xff;
		}
		switch(w & 0xff) {
		case PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE:
			info.imageType = &buffer[offset];
			break;
		case PICOBIN_BLOCK_ITEM_1BS_VERSION:
			info.version = &buffer[offset];
			break;
		case PICOBIN_BLOCK_ITEM_2BS_LAST:
			return info;
		}
		offset += len;
	}

	// Block not found or malformed
	return {};
}

bool PicoUpgrader::begin(Partition partition, size_t size)
{
	if(size > partition.size()) {
		return false; // too big
	}

	// RULES! Application images must be ota_xx type

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

	if(partition.type() == Storage::Partition::Type::app && stream->getWritePos() == 0) {
		// First sector: locate IMAGE_DEF word
		auto words = reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(buffer));
		assert(size >= HEADER_END_OFFSET);
		auto info = findImageDef(words + HEADER_START_OFFSET);

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

	// Read partition and determine state of TBYB flag, may need to rewrite sector

	SectorBuffer buffer;
	auto ok = partition.read(0, buffer, sizeof(buffer));
	debug_i("partition.read(%p, %u) %u", partition.address(), sizeof(buffer), ok);
	auto info = findImageDef(buffer + HEADER_START_OFFSET);
	if(!info) {
		debug_e("No IMAGE_DEF found in %s", partition.name().c_str());
		return false;
	}

	debug_i("Image flags 0x%04x, version %u.%u", info.getFlags(), info.getVersionMajor(), info.getVersionMinor());

	if(save) {
		// To ensure ROM code selects this partition for boot, image must have the highest version number

		// Determine highest version number for all other application images
		uint32_t maxVersion = 0;
		for(auto part : spiFlash->partitions()) {
			if(part.address() == partition.address()) {
				continue;
			}
			if(part.type() != Storage::Partition::Type::app) {
				continue;
			}
			HeaderBuffer hdr;
			part.read(HEADER_START_OFFSET * sizeof(uint32_t), hdr, sizeof(hdr));
			auto hdrInfo = findImageDef(hdr);
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

		return true;
	}

	auto flags = info.getFlags();
	if(!(flags & PICOBIN_IMAGE_TYPE_EXE_TBYB_BITS)) {
		info.setFlags(flags | PICOBIN_IMAGE_TYPE_EXE_TBYB_BITS);
		debug_i("Set flags 0x%04x", info.getFlags());
		partition.erase_range(0, INTERNAL_FLASH_SECTOR_SIZE);
		partition.write(0, buffer, sizeof(buffer));
	}

	// This won't actually reboot until application calls `system_restart`
	// since Sming resets watchdog in main loop
	int rc = rom_reboot(REBOOT2_FLAG_REBOOT_TYPE_FLASH_UPDATE, 5000, XIP_BASE + partition.address(), 0);
	return rc >= 0;
}

Partition PicoUpgrader::getBootPartition()
{
	// Scratch registers may indicate a vectored boot
	uint32_t addr = watchdog_hw->scratch[7];
	if(addr >= XIP_BASE && watchdog_hw->scratch[4] == VECTORED_BOOT_MAGIC &&
	   watchdog_hw->scratch[5] ^ VECTORED_BOOT_MAGIC == addr) {
		// Valid temporary boot
		addr -= XIP_BASE;
		return spiFlash->partitions().find(addr);
	}

	// Find application image with highest version
	Partition bootPart;
	uint32_t maxVersion = 0;
	for(auto part : spiFlash->partitions()) {
		if(part.type() != Storage::Partition::Type::app) {
			continue;
		}
		HeaderBuffer hdr;
		part.read(HEADER_START_OFFSET * sizeof(uint32_t), hdr, sizeof(hdr));
		auto info = findImageDef(hdr);
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
	// TODO
	boot_info_t boot_info{};
	int rc = rom_get_boot_info(&boot_info);
	if(rc < 0) {
		return {};
	}
	// rom_get
	return {};
}

} // namespace Ota
