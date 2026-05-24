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
#include <debug_progmem.h>

using namespace Storage;

namespace Ota
{
bool PicoUpgrader::begin(Partition partition, size_t size)
{
	if(size > partition.size()) {
		return false; // too big
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

	stream =
		std::make_unique<Storage::PartitionStream>(partition, 0, size ?: partition.size(), Storage::Mode::BlockErase);

	return true;
}

size_t PicoUpgrader::write(const uint8_t* buffer, size_t size)
{
	return stream ? stream->write(buffer, size) : 0;
}

bool PicoUpgrader::end()
{
	if(!stream) {
		return false;
	}

	stream.reset();

	// TODO
	return false;
}

bool PicoUpgrader::setBootPartition(Partition partition, bool save)
{
	// TODO
	return false;
}

Partition PicoUpgrader::getBootPartition()
{
	// TODO
	return {};
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
	for(auto part : Storage::findPartition()) {
		if(info.header.startOffset() == part.address()) {
			return part;
		}
	}

	return {};
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
