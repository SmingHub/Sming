/*
  IPAddress.cpp - Base class that provides IPAddress
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

#include <Arduino.h>
#include <IPAddress.h>
#include "WString.h"

IPAddress::IPAddress()
{
    memset(_address, 0, sizeof(_address));
}

IPAddress::IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet)
{
    _address[0] = first_octet;
    _address[1] = second_octet;
    _address[2] = third_octet;
    _address[3] = fourth_octet;
}

IPAddress::IPAddress(uint32_t address)
{
    memcpy(_address, &address, sizeof(_address));
}

IPAddress::IPAddress(ip_addr address)
{
    memcpy(_address, &address.addr, sizeof(_address));
}

IPAddress::IPAddress(const uint8_t *address)
{
    memcpy(_address, address, sizeof(_address));
}

void IPAddress::fromString(const String& address)
{
	int p = -1;
	for (int i = 0; i < 3; i++)
	{
		int prev = p + 1;
		p = address.indexOf('.', prev);
		if (p == -1)
		{
			debugf("WRONG IP: %s", address.c_str());
			break;
		}
		String sub = address.substring(prev, p);
		_address[i] = sub.toInt();
	}

	String sub = address.substring(p + 1);
	_address[3] = sub.toInt();
}

IPAddress::IPAddress(const String address)
{
	fromString(address);
}

IPAddress& IPAddress::operator=(const uint8_t *address)
{
    memcpy(_address, address, sizeof(_address));
    return *this;
}

IPAddress& IPAddress::operator=(uint32_t address)
{
    memcpy(_address, (const uint8_t *)&address, sizeof(_address));
    return *this;
}

IPAddress& IPAddress::operator=(const String address)
{
	fromString(address);
    return *this;
}

bool IPAddress::operator==(const uint8_t* addr)
{
    return memcmp(addr, _address, sizeof(_address)) == 0;
}

size_t IPAddress::printTo(Print& p) const
{
    size_t n = 0;
    for (int i =0; i < 3; i++)
    {
        n += p.print(_address[i], DEC);
        n += p.print('.');
    }
    n += p.print(_address[3], DEC);
    return n;
}

String IPAddress::toString()
{
	String res;
    for (int i =0; i < 3; i++)
    {
    	res.concat(_address[i]);
    	res.concat('.');
    }
    res.concat(_address[3]);
    return res;
}
