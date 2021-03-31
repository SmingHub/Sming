/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BitSet.h - Strongly typed sets of values
 *
 * @author: 2020 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <WString.h>

/**
 * @brief Manage a set of bit values using enumeration
 *
 * API is similar to a simplified std::bitset, but with added +/- operators.
 *
 * @tparam S Storage type (e.g. uint32_t).
 * This determines how much space to use, and must be an unsigned integer.
 * It is safe to use this class in structures, where it will occupy exactly the required space.
 *
 * @tparam E Element type e.g. enum class.
 * You can use any enum or unsigned integer for the elements. These must have ordinal sequence starting at 0.
 *
 * @tparam size_ Number of possible values in the set. Defaults to maximum for given storage type.
 * Number of possible values in the set. This must be at least 1, and cannot be more
 * than the given Storage type may contain.
 * For example, a :cpp:type:`uint8_t` may contain up to 8 values.
 *
 * @note It is important to specify **size** correctly when using enumerated values.
 * In the ``FruitBasket`` example, we use a ``uint8_t`` storage type so can have up to 8 possible values.
 * However, the ``Fruit`` enum contains only 7 values. The set operations will therefore be restricted to
 * ensure that the unused bit is never set.
 */
template <typename S, typename E, size_t size_ = sizeof(S) * 8> class BitSet
{
public:
	static_assert(std::is_integral<S>::value && std::is_unsigned<S>::value,
				  "BitSet requires an unsigned integral storage type");
	static_assert(std::is_enum<E>::value || (std::is_integral<E>::value && std::is_unsigned<E>::value),
				  "BitSets may use only enum or unsigned integral type elements");
	static_assert(size_ > 0, "BitSet size cannot be zero!");
	static_assert(size_ <= (sizeof(S) * 8), "BitSet has too many elements for storage type");

	class BitRef
	{
	public:
		operator bool() const
		{
			return bitset.test(e);
		}

		BitRef& operator=(bool b)
		{
			bitset.set(e, b);
			return *this;
		}

	private:
		friend BitSet;
		BitRef(BitSet& bitset, E e) : bitset(bitset), e(e)
		{
		}

		BitSet& bitset;
		E e;
	};

	/**
	 * @brief Construct empty set
	 */
	constexpr BitSet() = default;

	/**
	 * @brief Copy constructor
	 * @param bitset The set to copy
	 */
	template <typename S2> constexpr BitSet(const BitSet<S2, E>& bitset) : bitSetValue(bitset.value())
	{
	}

	/**
	 * @brief Construct from a raw set of bits
	 * @param value Integral type whose bits will be interpreted as set{E}
	 */
	constexpr BitSet(S value) : bitSetValue(value)
	{
	}

	/**
	 * @brief Construct set containing a single value
	 * @param e Value to place in our new BitSet object
	 */
	constexpr BitSet(E e) : bitSetValue{bitVal(e)}
	{
	}

	/**
	 * @brief Compare this set with another for equality
	 */
	bool operator==(const BitSet& other) const
	{
		return bitSetValue == other.bitSetValue;
	}

	/**
	 * @brief Compare this set with another for inequality
	 */
	bool operator!=(const BitSet& other) const
	{
		return bitSetValue != other.bitSetValue;
	}

	/**
	 * @brief Obtain a set containing all elements not in this one
	 */
	constexpr BitSet operator~() const
	{
		return BitSet(~bitSetValue & domain().bitSetValue);
	}

	/**
	 * @brief Get the number of possible elements in the set
	 */
	static constexpr size_t size()
	{
		return size_;
	}

	/**
	 * @brief Get the set of all possible values
	 */
	static constexpr BitSet domain()
	{
		return std::numeric_limits<S>::max() >> ((sizeof(S) * 8) - size_);
	}

	/**
	 * @brief Get the bitmask corresponding to a given value
	 */
	static constexpr S bitVal(E e)
	{
		return S{1U} << unsigned(e);
	}

	/**
	 * @brief Flip all bits in the set
	 */
	BitSet& flip()
	{
		bitSetValue = ~bitSetValue & domain().bitSetValue;
		return *this;
	}

	/**
	 * @brief Flip state of the given bit
	 */
	BitSet& flip(E e)
	{
		bitSetValue ^= bitVal(e);
		return *this;
	}

	/**
	 * @brief Get the number of elements in the set, i.e. bits set to 1
	 */
	size_t count() const
	{
		return __builtin_popcount(bitSetValue);
	}

	/**
	 * @brief Union: Add elements to set
	 */
	BitSet& operator+=(const BitSet& rhs)
	{
		bitSetValue |= rhs.bitSetValue;
		return *this;
	}

	/**
	 * @brief Remove elements from set
	 */
	BitSet& operator-=(const BitSet& rhs)
	{
		bitSetValue &= ~rhs.bitSetValue;
		return *this;
	}

	/**
	 * @brief Intersection: Leave only elements common to both sets
	 */
	BitSet& operator&=(const BitSet& rhs)
	{
		bitSetValue &= rhs.bitSetValue;
		return *this;
	}

	/**
	 * @brief Union: Add elements to set
	 */
	BitSet& operator|=(const BitSet& rhs)
	{
		bitSetValue |= rhs.bitSetValue;
		return *this;
	}

	/**
	 * @brief XOR - toggle state of bits using another set
	 */
	BitSet& operator^=(const BitSet& rhs)
	{
		bitSetValue ^= rhs.bitSetValue;
		return *this;
	}

	/**
	 * @brief Test to see if given element is in the set
	 */
	bool test(E e) const
	{
		return (bitSetValue & bitVal(e)) != 0;
	}

	/**
	 * @brief Read-only [] operator
	 * @param e Element to test for
	 * @retval bool true if given element is in the set
	 */
	bool operator[](E e) const
	{
		return test(e);
	}

	/**
	 * @brief Read/write [] operator
	 * @param e Element to read or write
	 * @retval BitRef Temporary object used to do the read or write
	 *
	 * This returns a temporary BitRef object to support assignment operations such as `set[x] = value`
	 */
	BitRef operator[](E e)
	{
		return BitRef{*this, e};
	}

	/**
	 * @brief Determine if set contains any values
	 */
	bool any() const
	{
		return bitSetValue != 0;
	}

	/**
	 * @brief Determine if set contains any values from another set
	 * i.e. intersection != []
	 */
	bool any(const BitSet& other) const
	{
		return (bitSetValue & other.bitSetValue) != 0;
	}

	/**
	 * @brief Test if set contains all possible values
	 */
	bool all() const
	{
		return bitSetValue == domain().bitSetValue;
	}

	/**
	 * @brief Test if set is empty
	 */
	bool none() const
	{
		return bitSetValue == S{0};
	}

	/**
	 * @brief Add all possible values to the bit set
	 */
	BitSet& set()
	{
		bitSetValue = domain().bitSetValue;
		return *this;
	}

	/**
	 * @brief Set the state of the given bit (i.e. add to or remove from the set)
	 * @param e Element to change
	 * @param state true to add the element, false to remove it
	 */
	BitSet& set(E e, bool state = true)
	{
		if(state) {
			bitSetValue |= bitVal(e);
		} else {
			bitSetValue &= ~bitVal(e);
		}
		return *this;
	}

	/**
	 * @brief Remove all values from the set
	 */
	BitSet& reset()
	{
		bitSetValue = S{0};
		return *this;
	}

	/**
	 * @brief Clear the state of the given bit (i.e. remove it from the set)
	 */
	BitSet& reset(E e)
	{
		return set(e, false);
	}

	/**
	 * @brief Determine if set consists of only the one given element
	 */
	bool operator==(E e) const
	{
		return bitSetValue == bitVal(e);
	}

	/**
	 * @brief Allow casts from the native storage type to get a numeric result for this set
	 */
	explicit constexpr operator S() const
	{
		return bitSetValue;
	}

	/**
	 * @brief Get stored bits for this bitset
	 */
	constexpr S value() const
	{
		return bitSetValue;
	}

	/*
	 * use a function pointer to allow for "if (s)" without the
	 * complications of an operator bool(). for more information,
	 * see: http://www.artima.com/cppsource/safebool.html
	 */
	using IfHelperType = void (BitSet::*)() const;

	/*
     * Provides safe bool() operator
     * Evaluates as false if set is empty
     */
	operator IfHelperType() const
	{
		return any() ? &BitSet::IfHelper : 0;
	}

private:
	void IfHelper() const
	{
	}

	S bitSetValue{0};
};

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator&(const BitSet<S, E, size_>& x, const BitSet<S, E, size_>& y)
{
	return BitSet<S, E, size_>(S(x) & S(y));
}

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator|(BitSet<S, E, size_> x, BitSet<S, E, size_> y)
{
	return BitSet<S, E, size_>(S(x) | S(y));
}

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator|(BitSet<S, E, size_> x, E b)
{
	return x | BitSet<S, E, size_>(b);
}

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator+(const BitSet<S, E, size_>& x, const BitSet<S, E, size_>& y)
{
	return x | y;
}

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator-(const BitSet<S, E, size_>& x, const BitSet<S, E, size_>& y)
{
	return BitSet<S, E, size_>(S(x) & ~S(y));
}

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator+(const BitSet<S, E, size_>& x, E b)
{
	return x | b;
}

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator-(const BitSet<S, E, size_>& x, E b)
{
	return BitSet<S, E, size_>(S(x) & ~BitSet<S, E, size_>::bitVal(b));
}

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator^(BitSet<S, E, size_> x, BitSet<S, E, size_> y)
{
	return BitSet<S, E, size_>(S(x) ^ S(y));
}

template <typename S, typename E, size_t size_>
inline constexpr BitSet<S, E, size_> operator^(BitSet<S, E, size_> x, E b)
{
	return x ^ BitSet<S, E, size_>(b);
}

/*
 * These allow construction of a maximally-sized BitSet in an expression,
 * which is then copy-constructed to the actual value. For example:
 *
 * 		constexpr BitSet<uint8_t, Fruit> fixedBasket = Fruit::apple | Fruit::orange;
 *
 * `is_convertible` prevents match to regular enums or integral values, but allows enum class.
 *
 */
template <typename E>
constexpr
	typename std::enable_if<std::is_enum<E>::value && !std::is_convertible<E, int>::value, BitSet<uint32_t, E>>::type
	operator|(E a, E b)
{
	return BitSet<uint32_t, E>(BitSet<uint32_t, E>::bitVal(a) | BitSet<uint32_t, E>::bitVal(b));
}

template <typename E>
constexpr
	typename std::enable_if<std::is_enum<E>::value && !std::is_convertible<E, int>::value, BitSet<uint32_t, E>>::type
	operator+(E a, E b)
{
	return a | b;
}

template <typename T> typename std::enable_if<std::is_integral<T>::value, String>::type toString(T value)
{
	return String(value);
}

/**
 * @brief Class template to print the contents of a BitSet to a String
 * @note Requires an implementation of `toString(E)`
 */
template <typename S, typename E, size_t size_>
String toString(const BitSet<S, E, size_>& bitset, const String& separator = ", ")
{
	extern String toString(E e);

	String s = String::empty;

	for(unsigned i = 0; i < bitset.size(); ++i) {
		if(!bitset[E(i)]) {
			continue;
		}

		if(s.length() != 0) {
			s += separator;
		}
		s += toString(E(i));
	}

	return s;
}

/**
 * @brief A set of 32 bits
 */
using BitSet32 = BitSet<uint32_t, uint8_t>;
