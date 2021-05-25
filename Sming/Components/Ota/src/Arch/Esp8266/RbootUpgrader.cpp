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

using namespace Storage;

namespace Ota
{
bool RbootUpgrader::begin(Partition partition, size_t size)
{
	if(partition.size() < size) {
		return false; // the requested size is too big...
	}

	status = rboot_write_init(partition.address());

	maxSize = size ?: partition.size();

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

bool RbootUpgrader::setBootPartition(Partition partition)
{
	return rboot_set_current_rom(getSlotForPartition(partition));
}

Partition RbootUpgrader::getBootPartition(void)
{
	return getPartitionForSlot(rboot_get_current_rom());
}

Partition RbootUpgrader::getRunningPartition(void)
{
	// TODO: ...
	return getPartitionForSlot(rboot_get_current_rom());
}

Partition RbootUpgrader::getNextBootPartition(Partition startFrom)
{
	uint8_t currentSlot = rboot_get_current_rom();
	return getPartitionForSlot(currentSlot ? 0 : 1);
}

uint8_t RbootUpgrader::getSlotForPartition(Partition partition)
{
	return (partition.subType() == uint8_t(Partition::SubType::App::ota1)) ? 1 : 0;
}

Partition RbootUpgrader::getPartitionForSlot(uint8_t slot)
{
	return *findPartition(Partition::apptypeOta(slot));
}

} // namespace Ota
