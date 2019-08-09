/*
  IpAddress.h - Base class that provides IP Address
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

#pragma once

#include <user_config.h>
#include "Printable.h"
#include "WString.h"

#if LWIP_VERSION_MAJOR == 2
#define LWIP_IP_ADDR_T const ip_addr_t
#else
typedef struct ip_addr ip_addr_t;
#define LWIP_IP_ADDR_T ip_addr_t
#endif

// A class to make it easier to handle and pass around IP addresses

class IpAddress : public Printable
{
private:
	ip_addr_t address = {0}; ///< IPv4 address

	void fromString(const String& address);

public:
	// Constructors
	IpAddress()
	{
	}

	IpAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet)
	{
		IP4_ADDR(&address, first_octet, second_octet, third_octet, fourth_octet);
	}

	IpAddress(uint32_t address)
	{
		this->address.addr = address;
	}

	IpAddress(ip_addr address)
	{
		this->address.addr = address.addr;
	}

#if LWIP_VERSION_MAJOR == 2
	IpAddress(ip_addr_t address)
	{
		this->address = address;
	}
#endif

	IpAddress(const uint8_t* address)
	{
		IP4_ADDR(&this->address, address[0], address[1], address[2], address[3]);
	}

	IpAddress(const String& address)
	{
		fromString(address);
	}

	// Overloaded cast operator to allow IpAddress objects to be used where a pointer
	// to a four-byte uint8_t array is expected
	operator uint32_t() const
	{
		return address.addr;
	}

	operator ip_addr() const
	{
		return {address.addr};
	}

	operator ip_addr*()
	{
		return reinterpret_cast<ip_addr*>(&address);
	}

#if LWIP_VERSION_MAJOR == 2
	operator ip_addr_t*()
	{
		return &address;
	}
#endif

	bool operator==(const IpAddress& addr) const
	{
		return ip_addr_cmp(&address, &addr.address);
	}

	bool operator==(const uint8_t* addr) const
	{
		return *this == IpAddress(addr);
	}

	bool operator!=(const IpAddress& addr) const
	{
		return !ip_addr_cmp(&address, &addr.address);
	}

	bool operator!=(const uint8_t* addr) const
	{
		return *this != IpAddress(addr);
	}

	bool isNull() const
	{
		return ip_addr_isany(&address);
	}

	String toString() const;

	bool compare(const IpAddress& addr, const IpAddress& mask) const
	{
		return ip_addr_netcmp(&address, &addr.address, &mask.address);
	}

	// Overloaded index operator to allow getting and setting individual octets of the address
	uint8_t operator[](int index) const
	{
		if(unsigned(index) >= sizeof(address)) {
			abort();
		}
		return reinterpret_cast<const uint8_t*>(&address)[index];
	}

	uint8_t& operator[](int index)
	{
		if(unsigned(index) >= sizeof(address)) {
			abort();
		}
		return reinterpret_cast<uint8_t*>(&address)[index];
	}

	// Overloaded copy operators to allow initialisation of IpAddress objects from other types
	IpAddress& operator=(const uint8_t* address)
	{
		IP4_ADDR(&this->address, address[0], address[1], address[2], address[3]);
		return *this;
	}

	IpAddress& operator=(uint32_t address)
	{
		this->address.addr = address;
		return *this;
	}

	IpAddress& operator=(const String address)
	{
		fromString(address);
		return *this;
	}

	size_t printTo(Print& p) const override;
};

/** @deprecated Use `IpAddress` instead. */
typedef IpAddress IPAddress SMING_DEPRECATED;

// Making this extern saves 100's of bytes; each usage otherwise incurs 4 bytes of BSS
#define INADDR_NONE IpAddress()
