/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * UpgradeOutputStream.cpp
 *
 *
 */

#include "include/Ota/UpgradeOutputStream.h"

namespace Ota
{

bool UpgradeOutputStream::init()
{
	bool success = ota.begin(partition, maxLength);
	initialized = true;

	return success;
}

size_t UpgradeOutputStream::write(const uint8_t* data, size_t size)
{
	if(!initialized && size > 0) {
		if(!init()) { // unable to initialize
			return 0;
		}

		initialized = true;
	}

	if(written + size > maxLength) {
		debug_e("The ROM size is bigger than the maximum allowed");
		return 0;
	}

	if(!ota.write(data, size)) {
		debug_e("ota_write_flash: Failed. Size: %d", size);
		return 0;
	}

	written += size;

	debug_d("ota_write_flash: item.size: %d", written);

	return size;
}

bool UpgradeOutputStream::close()
{
	if(initialized) {
		return ota.end();
	}

	return true;
}

} // namespace Ota
