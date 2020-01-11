/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ByteArray.h
 *
 ****/

#pragma once

#include "HashEngine.h"
#include <WString.h>
#include <Data/HexString.h>

namespace Crypto
{
/**
 * @brief Class template for a Hash value, essentially just an array of bytes
 */
template <class Engine_> struct HashValue {
	using Engine = Engine_;
	uint8_t data[Engine::size];

	static constexpr size_t size = sizeof(data);

	/**
	 * @brief Copy from a memory buffer
	 * @note Avoid using this method if possible as there are no checks on the data size
	 */
	bool copy(const void* src)
	{
		if(src == nullptr) {
			memset(data, 0, size);
			return false;
		}

		memcpy(data, src, size);
		return true;
	}

	bool operator==(const HashValue& other) const
	{
		return memcmp(data, other.data, size) == 0;
	}

	String toString(char separator = '\0') const
	{
		return makeHexString(data, size, separator);
	}
};

template <class Engine> String toString(const HashValue<Engine>& hash, char separator = '\0')
{
	return hash.toString(separator);
}

} // namespace Crypto
