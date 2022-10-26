/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Types.h
 *
 ****/
#pragma once

#include <cstdint>

#ifdef ENABLE_STORAGE_SIZE64
using storage_size_t = uint64_t;
#else
using storage_size_t = uint32_t;
#endif

namespace Storage
{
/**
 * @brief Determine if a value requires 64-bits to store
 */
inline bool isSize64(uint64_t value)
{
	using Lim = std::numeric_limits<uint32_t>;
	return value < Lim::min() || value > Lim::max();
}

/**
 * @brief Determine if a value requires 64-bits to store
 */
inline bool isSize64(int64_t value)
{
	using Lim = std::numeric_limits<int32_t>;
	return value < Lim::min() || value > Lim::max();
}

} // namespace Storage
