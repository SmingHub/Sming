/**
 * Uuid.cpp
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming SSDP Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "Uuid.h"
#include <SystemClock.h>
#include <stringconversion.h>

extern "C" uint32_t os_random();

bool Uuid::generate(MacAddress mac)
{
	uint8_t version = 1; // DCE version
	uint8_t variant = 2; // DCE variant
	uint16_t clock_seq = os_random();
	uint32_t time;
	if(SystemClock.isSet()) {
		time = SystemClock.now(eTZ_UTC);
	} else {
		time = os_random();
	}
	// Time only provides 32 bits, we need 60
	time_low = (os_random() & 0xFFFFFFFC) | (time & 0x00000003);
	time_mid = (time >> 2) & 0xFFFF;
	time_hi_and_version = (version << 12) | ((time >> 18) << 2);
	clock_seq_hi_and_reserved = (variant << 6) | ((clock_seq >> 8) & 0x3F);
	clock_seq_low = clock_seq & 0xFF;
	mac.getOctets(node);

	return SystemClock.isSet();
}

bool Uuid::decompose(const char* s, size_t len)
{
	if(len != stringSize) {
		return false;
	}

	char* p;
	time_low = strtoul(s, &p, 16);
	if(*p != '-' || p - s != 8) {
		return false;
	}
	s = ++p;

	time_mid = strtoul(s, &p, 16);
	if(*p != '-' || p - s != 4) {
		return false;
	}
	s = ++p;

	time_hi_and_version = strtoul(s, &p, 16);
	if(*p != '-' || p - s != 4) {
		return false;
	}
	s = ++p;

	uint16_t x = strtoul(s, &p, 16);
	if(*p != '-' || p - s != 4) {
		return false;
	}
	clock_seq_hi_and_reserved = x >> 8;
	clock_seq_low = x & 0xff;
	s = ++p;

	for(unsigned i = 0; i < sizeof(node); ++i) {
		uint8_t c = unhex(*s++) << 4;
		c |= unhex(*s++);
		node[i] = c;
	}

	return true;
}

size_t Uuid::toString(char* buffer, size_t bufSize) const
{
	if(buffer == nullptr || bufSize < stringSize) {
		return 0;
	}

	auto set = [&](unsigned offset, uint32_t value, unsigned digits) {
		ultoa_wp(value, &buffer[offset], 16, digits, '0');
	};

	// 2fac1234-31f8-11b4-a222-08002b34c003
	// 0        9    14   19   24          36

	set(0, time_low, 8);
	buffer[8] = '-';
	set(9, time_mid, 4);
	buffer[13] = '-';
	set(14, time_hi_and_version, 4);
	buffer[18] = '-';
	set(19, clock_seq_hi_and_reserved, 2);
	set(21, clock_seq_low, 2);
	buffer[23] = '-';

	unsigned pos = 24;
	for(unsigned i = 0; i < 6; ++i) {
		buffer[pos++] = hexchar(node[i] >> 4);
		buffer[pos++] = hexchar(node[i] & 0x0f);
	}

	return stringSize;
}

String Uuid::toString() const
{
	String s;
	if(s.setLength(stringSize)) {
		toString(s.begin(), stringSize);
	}
	return s;
}
