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

   http://www.hyperrealm.com/oss_commoncpp.shtml
*/

#pragma once

#include "WString.h"
#include <stdint.h>

/**
 * @brief A network hardware (MAC) address.
 * @author Mark Lindner
 * @author mikee47 <mike@sillyhouse.net>
 * 	Sming integration
 */

class MACAddress
{
	// https://www.artima.com/cppsource/safebool.html
	typedef void (MACAddress::*bool_type)() const;
	void Testable() const
	{
	}

public:
	MACAddress() = default;

	MACAddress(const uint8_t octets[6])
	{
		setOctets(octets);
	}

	/**
	 * @brief Get the octets of the MAC address.
	 */
	void getOctets(uint8_t octets[6]) const
	{
		memcpy(octets, this->octets, 6);
	}

	/**
	 * @brief Set the octets of the MAC address.
	 */
	void setOctets(const uint8_t octets[6])
	{
		memcpy(this->octets, octets, 6);
	}

	/**
	 * @brief Get the octet at the given index in the MAC address.
	 * @param index The index.
	 * @return The octet at the given index.
	 */
	uint8_t operator[](unsigned index) const;

	/**
	 * @brief Get a reference to the octet at the given index in the MAC address.
	 * @param index The index.
	 * @return A reference to the octet at the given index.
	 */
	uint8_t& operator[](unsigned index);

	/**
	 * @brief Return a String representation of the MACAddress.
	 */
	String toString(char sep = ':') const;

	/**
	 * @brief Equality operator.
	 */
	bool operator==(const MACAddress& other) const
	{
		return memcmp(octets, other.octets, sizeof(octets)) == 0;
	}

	/**
	 * @brief Inequality operator.
	 */
	inline bool operator!=(const MACAddress& other) const
	{
		return !operator==(other);
	}

	/**
	 * @brief Test if this is a null address (00:00:00:00:00:00).
	 */
	bool operator!() const;

	/**
	 * @brief Safe bool operator, returns true if address is non-null
	 */
	operator bool_type() const
	{
		return operator!() ? nullptr : &MACAddress::Testable;
	}

	/**
     * @brief Clear address to null value
     */
	void clear()
	{
		memset(octets, 0, sizeof(octets));
	}

	/**
	 * @brief Generate a simple 32-bit hash from the MAC address
	 * @retval uint32_t The hash
	 * @note This does not uniquely identify the key
	 */
	uint32_t getHash() const;

private:
	uint8_t octets[6] = {0};
};

#define MACADDR_NONE MACAddress()
