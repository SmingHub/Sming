/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Range.h
 *
 ****/

#pragma once

#include <WString.h>
#include <limits>
#include <esp_systemapi.h>

/**
 * @brief Manage a range of numbers between specified limits
 *
 * Values in the range meet the criteria (min <= value <= max)
 */
template <typename T> struct TRange {
	T min{};
	T max{};

	class Iterator
	{
	public:
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::random_access_iterator_tag;

		Iterator(T value) : value(value)
		{
		}

		T operator*() const
		{
			return value;
		}

		bool operator==(const Iterator& other) const
		{
			return value == other.value;
		}

		bool operator!=(const Iterator& other) const
		{
			return !(*this == other);
		}

		Iterator operator++(int)
		{
			Iterator ret(value);
			++*this;
			return ret;
		}

		Iterator& operator++()
		{
			++value;
			return *this;
		}

	private:
		T value;
	};

	constexpr TRange() = default;

	constexpr TRange(T min, T max) : min(min), max(max)
	{
	}

	constexpr TRange(T count) : min(0), max(count - 1)
	{
	}

	/**
	 * @brief Determine if range contains a value
	 */
	template <typename V> constexpr bool contains(V value) const
	{
		return (value >= min) && (value <= max);
	}

	/**
	 * @brief Determine if range contains another range (subset)
	 */
	template <typename Q> bool contains(const TRange<Q>& value) const
	{
		return contains(value.min) && contains(value.max);
	}

	/**
	 * @brief Clip values to within the range
	 */
	template <typename V> constexpr T clip(V value) const
	{
		return (value < min) ? min : (value > max) ? max : T(value);
	}

	/**
	 * @brief Return a random value within the range
	 */
	T random() const
	{
		uint64_t n = 1 + max - min;
		if(n == 0) {
			return 0;
		}
		T value = os_random();
		if(n > std::numeric_limits<uint32_t>::max()) {
			value |= uint64_t(os_random()) << 32;
		}
		return min + value % n;
	}

	Iterator begin() const
	{
		return Iterator{min};
	}

	Iterator end() const
	{
		return Iterator{T(max + 1)};
	}

	String toString() const
	{
		String s;
		s += min;
		s += ", ";
		s += max;
		return s;
	}

	operator String() const
	{
		return toString();
	}
};

template <typename T> inline String toString(TRange<T> range)
{
	return range.toString();
}
