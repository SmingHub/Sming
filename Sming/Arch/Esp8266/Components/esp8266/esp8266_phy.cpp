/*
 Adapted from Arduino for Sming.
 Original copyright note is kept below.

 phy.c - ESP8266 PHY initialization data
 Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

// #ifdef ENABLE_CUSTOM_PHY

#include "include/esp_phy.h"
#include <Storage.h>
#include <esp_attr.h>
#include <sys/pgmspace.h>
#include <string.h>

extern "C" {
extern int __wrap_register_chipv6_phy(uint8_t* initData);
extern int __real_register_chipv6_phy(uint8_t* initData);
}

int ICACHE_RAM_ATTR __wrap_register_chipv6_phy(uint8_t* initData)
{
	if(initData != NULL) {
		PhyInitData data{initData};
		customPhyInit(data);
	}
	return __real_register_chipv6_phy(initData);
}

// #endif /* ENABLE_CUSTOM_PHY */
