/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RbootUpgrader.cpp
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

bool RbootUpgrader::setBootPartition(Partition partition, bool save)
{
	uint8_t slot = getSlotForPartition(partition);
	if(!save) {
#ifdef BOOT_RTC_ENABLED
		return rboot_set_temp_rom(slot);
#else
		return false;
#endif
	}

	return rboot_set_current_rom(slot);
}

Partition RbootUpgrader::getRunningPartition()
{
	uint8_t slot = rboot_get_current_rom();
#ifdef BOOT_RTC_ENABLED
	rboot_rtc_data rtc;
	if(rboot_get_rtc_data(&rtc) && rtc.last_mode == MODE_TEMP_ROM) {
		slot = rtc.last_rom;
	}
#endif
	return getPartitionForSlot(slot);
}

} // namespace Ota
