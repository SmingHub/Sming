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

	BitSet() = default;
	BitSet(const BitSet&) = default;

	BitSet(E e) : value{bitVal(e)}
	{
	}

	BitSet& operator+=(BitSet rhs)
	{
		value |= rhs.value;
		return *this;
	}

	BitSet& operator-=(BitSet rhs)
	{
		value &= ~rhs.value;
		return *this;
	}

	BitSet& operator&=(const BitSet& rhs)
	{
		value &= rhs.value;
		return *this;
	}

	BitSet& operator|=(const BitSet& rhs)
	{
		value |= rhs.value;
		return *this;
	}

	BitSet& operator^=(const BitSet& rhs)
	{
		value ^= rhs.value;
		return *this;
	}

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

	bool any() const
	{
		return value != 0;
	}

	bool any(const BitSet& states) const
	{
		return (value & states.value) != 0;
	}

	bool all(const BitSet& states) const
	{
		return (value & states.value) == states.value;
	}

	bool none() const
	{
		return value == 0;
	}

	bool operator==(E e) const
	{
		return value == bitVal(e);
	}

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

using BitSet32 = BitSet<uint32_t, uint8_t>;
