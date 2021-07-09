/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Range.h
 *
 ****/

#include <WString.h>
#include <esp_systemapi.h>

/**
 * @brief Manage a range of numbers between specified limits
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

	constexpr TRange()
	{
	}

	constexpr TRange(T min, T max) : min(min), max(max)
	{
	}

	constexpr TRange(T count) : min(0), max(count - 1)
	{
	}

	bool contains(T value)
	{
		return (value >= min) && (value <= max);
	}

	T random() const
	{
		auto value = os_random();
		return min + value % (max - min);
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
};
