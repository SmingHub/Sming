/****
 * Uuid.h - Universal Unique Identifier
 *
 * See https://pubs.opengroup.org/onlinepubs/9629399/apdxa.htm.
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

#pragma once

#include <WString.h>
#include <MacAddress.h>

struct Uuid {
	uint32_t time_low{0};				  // 0-3
	uint16_t time_mid{0};				  // 4-5
	uint16_t time_hi_and_version{0};	  // 6-7, version = top 4 bits
	uint8_t clock_seq_hi_and_reserved{0}; // 8, variant = top 2 bits
	uint8_t clock_seq_low{0};			  // 9
	uint8_t node[6]{};					  // 10-15

	/**
	 * @brief Number of characters in a UUID string (excluding NUL terminator)
	 */
	static constexpr size_t stringSize = 36;

	Uuid()
	{
	}

	explicit Uuid(const char* s)
	{
		decompose(s);
	}

	explicit Uuid(const char* s, size_t len)
	{
		decompose(s, len);
	}

	explicit Uuid(const String& s)
	{
		decompose(s.c_str(), s.length());
	}

	explicit operator bool()
	{
		Uuid Null{};
		return memcmp(this, &Null, sizeof(Null)) != 0;
	}

	/**
	 * @note System clock must be set or this will not produce correct results.
	 */
	bool generate(MacAddress mac);

	bool decompose(const char* s, size_t len);

	bool decompose(const char* s)
	{
		return s ? decompose(s, strlen(s)) : false;
	}

	bool decompose(const String& s)
	{
		return decompose(s.c_str(), s.length());
	}

	/**
	 * @note Get string representation of UUID
	 * @param uuid
	 * @param buffer
	 * @param bufSize Must be at least UUID_STRING_SIZE
	 * @retval size_t number of characters written (either 0 or UUID_STRING_SIZE)
	 * @note Converts UUID into a string of the form
	 *
	 * <time_low>-<time_mid>-<time_high_and_version>-<clock_seq_and_reserved><clock_seq_low>-<node>
	 *
	 * e.g. 2fac1234-31f8-11b4-a222-08002b34c003
	 */
	size_t toString(char* buffer, size_t bufSize) const;

	String toString() const;

	operator String() const
	{
		return toString();
	}
};

inline String toString(const Uuid& uuid)
{
	return uuid.toString();
}

inline bool fromString(const char* s, Uuid& uuid)
{
	return uuid.decompose(s);
}

inline bool fromString(const String& s, Uuid& uuid)
{
	return uuid.decompose(s);
}

/**
 * @deprecated Use `Uuid` instead.
 */
typedef Uuid UUID SMING_DEPRECATED;
