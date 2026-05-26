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
#include <hardware/watchdog.h>
#include <debug_progmem.h>

using namespace Storage;

namespace Ota
{
// In documentation but not in SDK headers
constexpr uint32_t VECTORED_BOOT_MAGIC = 0xb007c0d3;

/**
 * @brief Determine the running or boot partition
 * @param running true to get the partition actually running,
 * false to get image scheduled to boot next
 *
 * For running=false, what will the boot ROM choose to boot next?
 *
 * Scratch registers may indicate a vectored boot.
 * Otherwise we need to ask boot ROM.
 */
Partition getActivePartition(bool running)
{
	boot_info_t boot_info{};
	int rc = rom_get_boot_info(&boot_info);
	if(rc < 0) {
		return {};
	}
	auto addr = boot_info.reboot_params[0];
	if(running) {
		if(addr >= XIP_BASE) {
			// Temporary boot
			addr -= XIP_BASE;
		} else {
			Pico::PartitionInfo info;
			Pico::getPartitionInfo(info, boot_info.partition);
			addr = info.header.startOffset();
		}
	} else {
		// What will the boot ROM do next?
		addr = watchdog_hw->scratch[7];
		if(addr >= XIP_BASE && watchdog_hw->scratch[4] == VECTORED_BOOT_MAGIC &&
		   watchdog_hw->scratch[5] ^ VECTORED_BOOT_MAGIC == addr) {
			// Valid temporary boot
			addr -= XIP_BASE;
		} else {
			// Find first application image
			// If it's an OTA partition, use pick_ab
		}
	}
	return spiFlash->partitions().find(addr);
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
	imageDefOffset = 0;
	imageTypeFlags = 0;
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
		// TODO: imageDefOffset = ...

		// Set TBYB bit in type flags so image doesn't get picked up automatically
		// This may get reset in `setBootPartition`
		// TODO: ...

		// Modify  IMAGE_DEF item to 0xffffffff so it can be rewritten later without erasure
		// This invalidates the image to ensure it won't get picked up prematurely
		// We configure this appropriately in `setBootPartition`.
		// TODO: memset(buffer[imageDefOffset], 0xff, 4);
	}

	return stream->write(buffer, size);
}

bool PicoUpgrader::end()
{
	if(!stream) {
		return false;
	}

	if(imageDefOffset != 0) {
		// Fixup IMAGE_DEF, set as TBYB - `setBootPartition` may clear this later
		// TODO: ...
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

	if(this.partition.address() == partition.address()) {
		if(imageDefOffset == 0) {
			return false;
		}
		// TODO ...
	} else {
		// Read partition and determine state of TBYB flag, may need to rewrite sector

		/*
			Expected IMAGE_DEF block:

			d3 de ff ff			
			42 01 21 10		PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE
							Flags: 0x1021
			48 02 00 00		PICOBIN_BLOCK_ITEM_1BS_VERSION
			06 00 01 00		Major 0x0001, Minor 0x0006
			ff 03 00 00		PICOBIN_BLOCK_ITEM_2BS_LAST
			a4 86 00 00		(relative pointer to next block loop)
			79 35 12 ab		PICOBIN_BLOCK_MARKER_END
		*/
	}

	if(save) {
		/*
			To ensure ROM code selects this partition for next boot:

				1. image version must be higher than other App partitions, OR
				2. other app partitions must be invalidated

			Ideally we do not want other partitions erased by ROM code, so (1) is preferable.
			What we could do here is determine the highest version number for all other application images,
			then increment it and write it here.

			If the version number is already the largest, we do nothing.
		 */
		return false;
	}

	// When application calls `system_restart` a watchdog reset is triggered
	/*
		TODO

		The problem with this is that if the image is *not* TBYB then this change
		ends up being permanent.
		So we need to check the TBYB flag in the partition.
		Or maybe do some kind of in-situ modification.

		Which reminds me, does the boot ROM validate the full application image?
		If it's just the start then we should write the first sector as the final step in an update.
		Actually, all we need to do is find the metadata block start marker and set it to 0xffffffff
		during download. Then we just re-write it as the final step.

		Since we've found the metadata block, we can get the IMAGE_DEF item and from that the EXE_TBYB flag.
		See bootrom code `s_varm_api_explicit_buy`.
	*/
	auto addr = partition.address();
	watchdog_hw->scratch[4] = VECTORED_BOOT_MAGIC;
	watchdog_hw->scratch[5] = addr ^ -watchdog_hw->scratch[4];
	watchdog_hw->scratch[6] = 0;
	watchdog_hw->scratch[7] = addr;
	return true;
}

Partition PicoUpgrader::getBootPartition()
{
	return getActivePartition(false);
}

Partition PicoUpgrader::getRunningPartition()
{
	return getActivePartition(true);
}

Partition PicoUpgrader::getNextBootPartition(Partition startFrom)
{
	boot_info_t boot_info{};
	int rc = rom_get_boot_info(&boot_info);
	if(rc < 0) {
		return {};
	}
	// rom_get
	return {};
}

} // namespace Ota
