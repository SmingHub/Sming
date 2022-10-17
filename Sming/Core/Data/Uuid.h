/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Uuid.h - Universal Unique Identifier
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <WString.h>
#include <MacAddress.h>

/**
 * @brief Class for manipulating UUID (aka GUID) entities
 * 
 * UUID: Universally Unique IDentifier
 * GUID: Globally Unique IDentifier
 *
 * See https://pubs.opengroup.org/onlinepubs/9629399/apdxa.htm.
 */
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

	constexpr Uuid()
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

	explicit Uuid(const String& s) : Uuid(s.c_str(), s.length())
	{
	}

	explicit Uuid(const FlashString& s) : Uuid(String(s))
	{
	}

	explicit constexpr Uuid(uint32_t time_low, uint16_t time_mid, uint16_t time_hi_and_version,
							uint8_t clock_seq_hi_and_reserved, uint8_t clock_seq_low, uint8_t n1, uint8_t n2,
							uint8_t n3, uint8_t n4, uint8_t n5, uint8_t n6)
		: time_low(time_low), time_mid(time_mid), time_hi_and_version(time_hi_and_version),
		  clock_seq_hi_and_reserved(clock_seq_hi_and_reserved),
		  clock_seq_low(clock_seq_low), node{n1, n2, n3, n4, n5, n6}
	{
	}

	explicit operator bool() const
	{
		return *this != Uuid{};
	}

	bool operator==(const Uuid& other) const;

	bool operator!=(const Uuid& other) const
	{
		return !operator==(other);
	}

	/**
	 * @brief Generate a UUID using a MAC node address
	 * @param mac Node address to use in generating the UUID, typically from WifiStation
	 * @retval bool true if system clock time was used, false if substituted with random number
	 */
	bool generate(MacAddress mac);

	/**
	 * @brief Generate UUID using random number instead of MAC
	 * @retval bool true if system clock time was used, false if substituted with random number
	 *
	 * Used where MAC address is not available or it is not desirable to expose it.
	 */
	bool generate();

	/**
	 * @name Decompse string into UUID
	 * @{
	 */
	bool decompose(const char* s, size_t len);

	bool decompose(const char* s)
	{
		return s ? decompose(s, strlen(s)) : false;
	}

	bool decompose(const String& s)
	{
		return decompose(s.c_str(), s.length());
	}
	/** @} */

	/**
	 * @name Get string representation of UUID
	 * @{
	 */

	/**
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

	/** @} */
};

static_assert(sizeof(Uuid) == 16, "Bad Uuid");

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
