/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BitSet.h
 *
 * @author: 2020 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <stdint.h>

/**
 * @brief Manage a set of bit values using enumeration
 *
 * API is similar to a simplified std::bitset, but with added +/- operators.
 *
 * @tparam S Storage type (e.g. uint32_t)
 * @tparam E Element type e.g. enum class
 */
template <typename S, typename E> class BitSet
{
public:
	class BitRef
	{
	public:
		operator bool() const
		{
			return set.test(e);
		}

		BitRef& operator=(bool b)
		{
			if(b) {
				set += e;
			} else {
				set -= e;
			}
			return *this;
		}

	private:
		friend BitSet;
		BitRef(BitSet& set, E e) : set(set), e(e)
		{
		}

		BitSet& set;
		E e;
	};

	/**
	 * @brief Construct empty set
	 */
	BitSet() = default;

	/**
	 * @brief Copy constructor
	 */
	BitSet(const BitSet&) = default;

	/**
	 * @brief Construct set containing a single value
	 */
	BitSet(E e) : value{bitVal(e)}
	{
	}

	/**
	 * @brief Add value to set
	 */
	BitSet& operator+=(BitSet rhs)
	{
		value |= rhs.value;
		return *this;
	}

	/**
	 * @brief Remove value from set
	 */
	BitSet& operator-=(BitSet rhs)
	{
		value &= ~rhs.value;
		return *this;
	}

	/**
	 * @brief Intersection
	 */
	BitSet& operator&=(const BitSet& rhs)
	{
		value &= rhs.value;
		return *this;
	}

	/**
	 * @brief Union
	 */
	BitSet& operator|=(const BitSet& rhs)
	{
		value |= rhs.value;
		return *this;
	}

	/**
	 * @brief XOR - toggle state of bits using another set
	 */
	BitSet& operator^=(const BitSet& rhs)
	{
		value ^= rhs.value;
		return *this;
	}

	/**
	 * @name Test to see if given value is in the set
	 * @note Also allows assignment operations such as `set[x] = value`
	 * @{
	 */
	BitRef operator[](E e)
	{
		return BitRef{*this, e};
	}

	bool operator[](E e) const
	{
		return test(e);
	}

	bool test(E e) const
	{
		return (value & bitVal(e)) != 0;
	}

	/** @} */

	/**
	 * @brief Determine if set contains any values
	 */
	bool any() const
	{
		return value != 0;
	}

	/**
	 * @brief Determine if set contains any values from another set
	 * i.e. intersection != []
	 */
	bool any(const BitSet& states) const
	{
		return (value & states.value) != 0;
	}

	/**
	 * @brief Determine if two sets are identical
	 */
	bool all(const BitSet& states) const
	{
		return (value & states.value) == states.value;
	}

	/**
	 * @brief Determine if set is empty
	 */
	bool none() const
	{
		return value == 0;
	}

	/**
	 * @brief Determine if set contains one specific value
	 */
	bool operator==(E e) const
	{
		return value == bitVal(e);
	}

	/**
	 * @brief Allow casts from the native storage type to get a numeric result for this set
	 */
	explicit operator S() const
	{
		return value;
	}

private:
	S bitVal(E e) const
	{
		return S{1U} << unsigned(e);
	}

	S value{0};
};

template <typename S, typename E> inline BitSet<S, E> operator&(const BitSet<S, E>& x, const BitSet<S, E>& y)
{
	BitSet<S, E> r(x);
	r &= y;
	return r;
}

template <typename S, typename E> inline BitSet<S, E> operator|(const BitSet<S, E>& x, const BitSet<S, E>& y)
{
	BitSet<S, E> r(x);
	r |= y;
	return r;
}

template <typename S, typename E> inline BitSet<S, E> operator+(const BitSet<S, E>& x, const BitSet<S, E>& y)
{
	BitSet<S, E> r(x);
	r += y;
	return r;
}

template <typename S, typename E> inline BitSet<S, E> operator-(const BitSet<S, E>& x, const BitSet<S, E>& y)
{
	BitSet<S, E> r(x);
	r -= y;
	return r;
}

template <typename S, typename E> inline BitSet<S, E> operator+(const BitSet<S, E>& x, E y)
{
	auto r(x);
	r += y;
	return r;
}

template <typename S, typename E> inline BitSet<S, E> operator-(const BitSet<S, E>& x, E y)
{
	auto r(x);
	r -= y;
	return r;
}

/**
 * @brief A set of 32 bits
 */
using BitSet32 = BitSet<uint32_t, uint8_t>;
