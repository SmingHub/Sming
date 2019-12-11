/*
 Original code: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_features.cpp
 core_esp8266_features.cpp

 Copyright (c) 2019 Mike Nix. All rights reserved.
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

#include <stdint.h>

#pragma once

/**
 * @defgroup iram_precache IRAM Precache Support
 * @ingroup flash
 * @{
 */

/**
 * @brief Tools for pre-loading code into the flash cache
 *
 * - It can be useful for code that accesses/uses SPI0 which is connected to the flash chip.
 * - Non interrupt handler code that is infrequently called but might otherwise
 *	 require being in valuable IRAM - such as bit-banging I/O code or some code
 *	 run at bootup can avoid being permanently in IRAM.
 *
 */

/**
 * @brief Mark functions containing critical code using this attribute
 */
#define IRAM_PRECACHE_ATTR __attribute__((optimize("no-reorder-blocks"))) __attribute__((noinline))

/**
 * @brief Place this macro before the first line of the critical code
 * @param tag Used to create the precached section name, must be globally unique
 * @note Do not omit the tag, and be careful with naming to avoid conflicts
 */
#define IRAM_PRECACHE_START(tag)                                                                                       \
	iram_precache(NULL, (uint8_t*)&&iram_precache_end_##tag - (uint8_t*)&&iram_precache_start_##tag);                  \
	iram_precache_start_##tag:

/**
 * @brief Place this macro after the last line of critical code
 * @param tag Must be the same tag used in IRAM_PRECACHE_START()
 */
#define IRAM_PRECACHE_END(tag) iram_precache_end_##tag:

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Pre-load flash data into the flash instruction cache
 *  @param addr First location to cache, specify NULL to use current location.
 *  @param bytes Number of bytes to cache
 *  @note All pages containing the requested region will be read to pull them into cache RAM.
 */
void iram_precache(void* addr, uint32_t bytes);

/** @} */

#ifdef __cplusplus
}
#endif
