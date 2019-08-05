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

#include "MACAddress.h"
#include <Data/HexString.h>

uint8_t MACAddress::operator[](unsigned index) const
{
	if(index >= sizeof(octets)) {
		abort();
	}

	return octets[index];
}

uint8_t& MACAddress::operator[](unsigned index)
{
	if(index >= sizeof(octets)) {
		abort();
	}

	return octets[index];
}

String MACAddress::toString(char sep) const
{
	return makeHexString(octets, sizeof(octets), sep);
}

bool MACAddress::operator!() const
{
	return *this != MACADDR_NONE;
}

uint32_t MACAddress::getHash() const
{
	uint32_t a = octets[4] | (octets[5] << 8);
	uint32_t b = octets[0] | (octets[1] << 8) | (octets[2] << 16) | (octets[3] << 24);
	return a ^ b;
}
