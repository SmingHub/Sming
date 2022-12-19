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

/**
 * @name Get power of 2 for given value
 * @param value Must be an exact power of 2
 * @retval uint8_t Result n such that `value == 1 << n`
 * @see Use `isLog2()` to confirm value is power of 2
 * @{
 */
template <typename T> constexpr typename std::enable_if<(sizeof(T) <= 4), uint8_t>::type getSizeBits(T value)
{
	return __builtin_ffs(value) - 1;
}

template <typename T> constexpr typename std::enable_if<(sizeof(T) > 4), uint8_t>::type getSizeBits(T value)
{
	return __builtin_ffsll(value) - 1;
}
/** @} */

/**
 * @brief Determine if a value is an exact power of 2
 */
template <typename T> constexpr bool isLog2(T value)
{
	return value == (T(1U) << getSizeBits(value));
}

} // namespace Storage
