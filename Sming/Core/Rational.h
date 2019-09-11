/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Rational.h - Run-time rational number handling
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 * Where calculations all involve constant values use std::ratio to have computations performed at compile time.
 *
 ****/

#pragma once

#include <stdint.h>
#include <ratio>
#include <algorithm>
#include <muldiv.h>
#include <WString.h>

/**
 * @brief A basic rational fraction, constexpr-compatible
 * @note Adding a constructor would prevent things like this:
 *
 * 		constexpr BasicRatio ratio {1, 5};
 *
 * 	We can use BasicRatio to do things like creating tables of ratios,
 * 	which can then be used in std::ratio calls to perform compile-time arithmetic.
 */
template <typename T> struct BasicRatio {
	T num;
	T den;

	operator String() const
	{
		String s(num);
		s += '/';
		s += den;
		return s;
	}
};

using BasicRatio16 = BasicRatio<uint16_t>;
using BasicRatio32 = BasicRatio<uint32_t>;
using BasicRatio64 = BasicRatio<uint64_t>;

/**
 * @brief Class to simplify calculations of finite rationals at runtime
 * @note Operations are restricted to multiplication and division
 * using unsigned integer operations.
 * Multiplication and division are rounded to the nearest whole number.
 * Overflows are handled by returning the maximum value rather than truncating.
 * @note Addition and subtraction are not supported directly as this would require overflow
 * detection and probably signed arithmetic, which is slower and limits calculation range.
 * @see See `muldiv` for further details.
 * @note Ratio values are minimised in constructor
 */
template <typename T> struct Ratio : public BasicRatio<T> {
	Ratio() = default;

	Ratio(const BasicRatio<T>& r)
	{
		this->num = r.num;
		this->den = r.den;
	}

	explicit Ratio(T num, T den = 1)
	{
		set(num, den);
	}

	/**
	 * @brief Method template to set ratio and minimise
	 * @note Use this method where values are known at compile-time
	 */
	template <T num, T den> void set()
	{
		using R = std::ratio<num, den>;
		this->num = R::num;
		this->den = R::den;
	}

	/**
	 * @brief Set ratio and minimise
	 * @note Use this method for variable ratios
	 */
	void set(T num, T den = 1)
	{
		// Note: Officially added std::numerics::gcd in C++17
		auto g = std::__gcd(num, den);
		this->num = num / g;
		this->den = den / g;
	}

	template <typename ValueType> explicit operator ValueType() const
	{
		return muldiv(this->num, T(1), this->den);
	}

	/**
	 * @brief Evaluate `ratio = ratio1 * ratio2` and minimise
	 */
	friend Ratio operator*(const Ratio& lhs, const Ratio& rhs)
	{
		return Ratio(lhs.num * rhs.num, lhs.den * rhs.den);
	}

	/**
	 * @brief Evaluate `value = value * ratio`
	 */
	template <typename ValueType> friend ValueType operator*(ValueType lhs, const Ratio& rhs)
	{
		return muldiv(lhs, rhs.num, rhs.den);
	}

	/**
	 * @brief Evaluate `value = ratio * value`
	 */
	template <typename ValueType> friend ValueType operator*(const Ratio& lhs, const ValueType& rhs)
	{
		return rhs * lhs;
	}

	/**
	 * @brief Evaluate `ratio = ratio1 / ratio2` and minimise
	 */
	friend Ratio operator/(const Ratio& lhs, const Ratio& rhs)
	{
		return Ratio(lhs.num * rhs.den, lhs.den * rhs.num);
	}

	/**
	 * @brief Evaluate `value = Ratio / value`
	 */
	template <typename ValueType> friend Ratio operator/(Ratio lhs, const ValueType& rhs)
	{
		return Ratio(lhs.num, lhs.den * rhs);
	}

	/**
	 * @brief Evaluate `value = value / ratio`
	 */
	template <typename ValueType> friend ValueType operator/(ValueType lhs, const Ratio& rhs)
	{
		return muldiv(lhs, rhs.den, rhs.num);
	}
};

using Ratio16 = Ratio<uint16_t>;
using Ratio32 = Ratio<uint32_t>;
using Ratio64 = Ratio<uint64_t>;
