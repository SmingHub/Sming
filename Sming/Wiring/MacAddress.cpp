/* ---------------------------------------------------------------------------
   commonc++ - A C++ Common Class Library
   Copyright (C) 2005-2012  Mark A Lindner

   This file is part of commonc++.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   ---------------------------------------------------------------------------
*/

#include "MacAddress.h"
#include <Data/HexString.h>

MacAddress::MacAddress(const String& s)
{
	// Maximum length with 2 hex digits per octet plus optional separator
	bool sep;
	if(s.length() == 12) {
		sep = false;
	} else if(s.length() == 17) {
		sep = true;
	} else {
		return;
	}
	auto str = s.c_str();
	unsigned pos{0};
	Octets res{};
	for(unsigned i = 0; i < 6; ++i) {
		if(sep && i != 5 && strchr(_F(":-.,/ "), str[2]) == nullptr) {
			return;
		}
		if(!isxdigit(str[0]) || !isxdigit(str[1])) {
			return;
		}
		res[i] = (unhex(str[0]) << 4) | unhex(str[1]);
		str += 2 + unsigned(sep);
	}
	memcpy(octets, res, sizeof(res));
}

uint8_t& MacAddress::operator[](unsigned index)
{
	if(index >= sizeof(octets)) {
		abort();
	}

	return octets[index];
}

String MacAddress::toString(char sep) const
{
	return makeHexString(octets, sizeof(octets), sep);
}

bool MacAddress::operator!() const
{
	return *this == MACADDR_NONE;
}

uint32_t MacAddress::getHash() const
{
	uint32_t a = octets[4] | (octets[5] << 8);
	uint32_t b = octets[0] | (octets[1] << 8) | (octets[2] << 16) | (octets[3] << 24);
	return a ^ b;
}
