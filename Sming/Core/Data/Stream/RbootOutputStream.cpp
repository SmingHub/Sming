/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RbootOutputStream.cpp
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 */

#include "RbootOutputStream.h"

bool RbootOutputStream::init()
{
	rBootWriteStatus = rboot_write_init(startAddress);
	initialized = true;

	return true;
}

size_t RbootOutputStream::write(const uint8_t* data, size_t size)
{
	if(!initialized && size > 0) {
		if(!init()) { // unable to initialize
			return 0;
		}

		initialized = true;
	}

	if(maxLength && (written + size > maxLength)) {
		debug_e("The ROM size is bigger than the maximum allowed");
		return 0;
	}

	if(!rboot_write_flash(&rBootWriteStatus, (uint8_t*)data, size)) {
		debug_e("rboot_write_flash: Failed. Size: %d", size);
		return 0;
	}

	written += size;

	debug_d("rboot_write_flash: item.size: %d", written);

	return size;
}

bool RbootOutputStream::close()
{
	return rboot_write_end(&rBootWriteStatus);
}
