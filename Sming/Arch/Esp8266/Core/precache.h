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

/*
 * Tools for pre-loading code into the flash cache
 *
 * - It can be useful for code that accesses/uses SPI0 which is connected to the flash chip.
 * - Non interrupt handler code that is infrequently called but might otherwise
 *	 require being in valuable IRAM - such as bit-banging I/O code or some code
 *	 run at bootup can avoid being permanently in IRAM.
 *
 */

#define PRECACHE_ATTR __attribute__((optimize("no-reorder-blocks"))) \
                      __attribute__((noinline))

#define PRECACHE_START(tag) \
    precache(NULL,(uint8_t *)&&_precache_end_##tag - (uint8_t*)&&_precache_start_##tag); \
    _precache_start_##tag:

#define PRECACHE_END(tag) \
    _precache_end_##tag:

#ifdef __cplusplus
extern "C" {
#endif

void precache(void *f, uint32_t bytes);

#ifdef __cplusplus
}
#endif
