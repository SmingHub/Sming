/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * muldiv.h - Functions to perform unit conversion using unsigned integer arithmetic
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 * To perform conversion between different units requires the following calculation:
 *
 * 		result = value * ratio
 *
 * Where `ratio` would be our conversion factor, e.g. 0.04
 *
 * The main problem with floating point calculations is speed.
 *
 * Integer maths is much faster (unsigned integer maths are the fastest).
 * Instead of 0.04 we'd use a rational fraction 4/100.
 * generalised as num/den (or num:den, where num = numerator, den = denominator).
 *
 * The calculation then becomes:
 *
 * 		result = value * num / den
 *
 * However, the result would be truncated so a calculation like this:
 *
 * 		const unsigned value = 120;
 * 		const unsigned num = 4;
 * 		const unsigned den = 100;
 *
 * 		unsigned result = 120 * 4 / 100 = 480 / 100 = 4 (truncated) or 5 if rounded
 *
 * Generally we want the rounded result which we can do like this:
 *
 *   result = round(value * num / den)
 * 	        = (value * num + 0.5) / den
 *          = ((value * num) + (den / 2)) / den
 *
 * Note that calculation range is limited by the multiplication (and the addition),
 * so in practice we'd minimise the ratio to 1/25 in order to minimise overflow.
 * The template versions of muldiv do this automatically as it doesn't cost us anything.
 *
 ****/

#pragma once

#include <limits>
#include <ratio>
#include <esp_attr.h>
#include <sming_attr.h>

/**
 * @brief Obtain limits for a muldiv template calculation
 * @tparam num
 * @tparam den
 * @tparam ValType
 */
template <uint64_t num, uint64_t den, typename ValType> struct MuldivLimits {
	/**
	 * @brief Get the value representing overflow for the given ValType
	 * @retval ValType
	 * @note Value equivalent to ValType(-1)
	 */
	static constexpr ValType overflow()
	{
		return std::numeric_limits<ValType>::max();
	}

	/**
	 * @brief Get the maximum value which can be used for a muldiv calcuation without overflowing
	 * @retval ValType Values greater than this will return `overflow()`
	 */
	static constexpr ValType maxValue()
	{
		return ({
			using R = std::ratio<num, den>;
			constexpr auto frac = R::den / 2;
			(overflow() - frac) / R::num;
		});
	}
};

/**
 * @brief Get the maximum value which can be passed to muldiv() without overflowing
 * @tparam ValType
 * @param timevar Result depends on size of variable used
 * @param num
 * @param den
 * @retval ValType Values greater than this will cause muldiv to return ValType(-1), i.e. range maximum
 */
template <typename ValType> __forceinline ValType muldivMaxValue(ValType timevar, ValType num, ValType den)
{
	constexpr auto max = std::numeric_limits<ValType>::max();
	auto frac = den / 2;
	return ValType((max - frac) / num);
}

/**
 * @brief Perform muldiv using unsigned integer types
 * @tparam ValType
 * @tparam NumDenType
 * @param value
 * @param num
 * @param den
 * @retval ToType Returns numeric_limits<ToType>::max() on overflow (same as ValType(-1))
 */
template <typename ValType, typename NumDenType>
__forceinline ValType IRAM_ATTR muldiv(const ValType& value, const NumDenType& num, const NumDenType& den)
{
	static_assert(std::is_unsigned<ValType>::value && std::is_unsigned<NumDenType>::value,
				  "muldiv types must be unsigned");

	if(value == 0 || num == 0) {
		return 0;
	}

	constexpr auto max = std::numeric_limits<ValType>::max();

	if(den == 0) {
		return max;
	}

	// Adds 0.5 to result to round value to nearest integer
	auto frac = ValType(den / 2);

	/*
	 * calculation:	result = ((value * num) + frac) / den
	 * overflow:	((value * num) + frac) > max
	 * or:			value > (max - frac) / num
	 */
	const ValType lim = (max - frac) / num;
	if(value > lim) {
		if(sizeof(ValType) < sizeof(uint64_t)) {
			// Try using 64-bit calculation
			return muldiv(uint64_t(value), num, den);
		} else {
			return max; // overflow
		}
	}

	auto mul = num * value;

	if(frac == 0) {
		return mul;
	}

	return (mul + frac) / den;
}

/**
 * @brief Perform muldiv using 32-bit values
 * @param value
 * @param num
 * @param den
 * @retval uint32_t
 * @note Calculation will still use uint64_t internally if necessary to prevent overflow
 */
__forceinline uint32_t IRAM_ATTR muldiv32(uint32_t value, uint32_t num, uint32_t den)
{
	return muldiv(value, num, den);
}

/**
 * @brief Perform muldiv using 64-bit values
 * @param value
 * @param num
 * @param den
 * @retval uint32_t
 * @note If calculation overflows there is always floating point, but will probably require double precision
 */
__forceinline uint64_t IRAM_ATTR muldiv64(const uint64_t& value, const uint64_t& num, const uint64_t& den)
{
	return muldiv(value, num, den);
}

/**
 * @brief Templated muldiv version so numerator and denominator are pre-calculated
 * @tparam num
 * @tparam den
 * @tparam ValType
 * @param value
 * @retval ValType Returns numeric_limits<ValType>::max() on overflow (same as ValType(-1))
 */
template <uint64_t num, uint64_t den, typename ValType> __forceinline ValType IRAM_ATTR muldiv(const ValType& value)
{
	static_assert(std::is_unsigned<ValType>::value, "muldiv types must be unsigned");

	// Minimise the fraction
	using R = std::ratio<num, den>;
	// Adds 0.5 to result to round value to nearest integer
	constexpr ValType frac = R::den / 2;
	constexpr ValType max = std::numeric_limits<ValType>::max();

	if(value == 0 || R::num == 0) {
		return 0;
	}

	// Probably already checked by std::ratio
	if(R::den == 0) {
		return max;
	}

	/*
	 * calculation:	result = ((value * num) + frac) / den
	 * overflow:	((value * num) + frac) > max
	 * or:			value > (max - frac) / num
	 */

	constexpr ValType lim = (max - frac) / R::num;
	if(value > lim) {
		if(sizeof(ValType) < sizeof(uint64_t)) {
			// Try using 64-bit calculation
			return muldiv<R::num, R::den>(uint64_t(value));
		} else {
			return max; // overflow
		}
	}

	return ((value * ValType(R::num)) + frac) / ValType(R::den);
}
