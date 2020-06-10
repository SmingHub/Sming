/**
 * random.cpp - pseudo-random generator
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/esp_libc.h"
#include <chrono>
#include <assert.h>
#include <esp_system.h>

uint32_t os_random(void)
{
	return esp_random();
}

int os_get_random(uint8_t* buf, size_t len)
{
	if(len == 0) {
		return 0; // Success, but nothing generated
	}

	assert(buf != nullptr);

	uint32_t rand = 0;
	for(unsigned i = 0; i < len; ++i) {
		if(i % 4 == 0) {
			rand = os_random();
		}
		buf[i] = rand;
		rand >>= 8;
	}

	// Success
	return 0;
}
