/*
  IpAddress.cpp - Base class that provides IpAddress
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

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

#include "IpAddress.h"
#include "Print.h"

void IpAddress::fromString(const String& address)
{
	this->address.addr = 0;
	const char* p = address.c_str();
	for(unsigned i = 0; i < 4; ++i) {
		operator[](i) = strtol(p, const_cast<char**>(&p), 10);
		if (*p++ != '.')
			break;	// Missing '.' or end of input string
	}
}

size_t IpAddress::printTo(Print& p) const
{
    size_t n = 0;
    for (unsigned i = 0; i < 3; i++) {
        n += p.print(operator[](i), DEC);
        n += p.print('.');
    }
    n += p.print(operator[](3), DEC);
    return n;
}

String IpAddress::toString() const
{
	String res;
    res.reserve(sizeof(address) * 4);
    for (unsigned i = 0; i < sizeof(address); i++)
    {
    	if (i)
    	  res += '.';
    	res += operator[](i);
    }
    return res;
}
