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

#include <WString.h>
#include <Data/HexString.h>
#include <iterator>

namespace Crypto
{
/**
 * @brief Class template for fixed byte array
 * @note Until C++17 (and GCC > 5.5) inheriting from std::array<> breaks
 * aggregate initialization.
 */
template <size_t size_> using ByteArray = std::array<uint8_t, size_>;

template <size_t size_> String toString(const ByteArray<size_>& array, char separator = '\0')
{
	return makeHexString(array.data(), array.size(), separator);
}

} // namespace Crypto
