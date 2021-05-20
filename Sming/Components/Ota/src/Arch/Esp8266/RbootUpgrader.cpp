/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Ota.cpp
 *
 ****/

#include "include/Ota/RbootUpgrader.h"

namespace Ota
{
bool RbootUpgrader::begin(Storage::Partition partition, size_t size)
{
	if(partition.size() < size) {
		return false; // the requested size is too big...
	}

	status = rboot_write_init(partition.address());

	maxSize = (size ? size : partition.size());

	writtenSoFar = 0;

	return true;
}

size_t RbootUpgrader::write(const uint8_t* buffer, size_t size)
{
	if(writtenSoFar + size > maxSize) {
		// cannot write more bytes than allowed
		return 0;
	}

	if(!rboot_write_flash(&status, buffer, size)) {
		// write failed
		return 0;
	}

	writtenSoFar += size;

	return size;
}

bool RbootUpgrader::end()
{
	return rboot_write_end(&status);
}

bool RbootUpgrader::setBootPartition(Storage::Partition partition)
{
	return rboot_set_current_rom(getSlotForPartition(partition));
}

Storage::Partition RbootUpgrader::getBootPartition(void)
{
	return getPartitionForSlot(rboot_get_current_rom());
}

Storage::Partition RbootUpgrader::getRunningPartition(void)
{
	// TODO: ...
	return getPartitionForSlot(rboot_get_current_rom());
}

Storage::Partition RbootUpgrader::getNextBootPartition(Storage::Partition* startFrom)
{
	uint8_t currentSlot = rboot_get_current_rom();
	return getPartitionForSlot(currentSlot ? 0 : 1);
}

uint8_t RbootUpgrader::getSlotForPartition(Storage::Partition partition)
{
	if(partition.subType() == uint8_t(Storage::Partition::SubType::App::ota1)) {
		return 1;
	}

	return 0;
}

Storage::Partition RbootUpgrader::getPartitionForSlot(uint8_t slot)
{
	auto partitions = Storage::findPartition(
		Storage::Partition::Type::app,
		uint8_t(slot == 0 ? Storage::Partition::SubType::App::ota0 : Storage::Partition::SubType::App::ota1));

	return *partitions;
}

} // namespace Ota
