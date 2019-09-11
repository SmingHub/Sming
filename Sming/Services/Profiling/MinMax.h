/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MinMax.h
 *
 ****/

#pragma once

#include <WString.h>
#include <Printable.h>

namespace Profiling
{
/**
 * @brief Class to track minimum and maximum values of a set of data, with average, total and count
 */
template <typename T> class MinMax : public Printable
{
public:
	MinMax(const String& title) : title(title)
	{
	}

	void reset();

	void update(T value);

	T getMin() const
	{
		return minVal;
	}

	T getMax() const
	{
		return maxVal;
	}

	T getTotal() const
	{
		return total;
	}

	T getAverage() const;

	unsigned getCount() const
	{
		return count;
	}

	size_t printTo(Print& p) const override;

private:
	String title;
	unsigned count = 0;
	T total = 0;
	T minVal = 0;
	T maxVal = 0;
};

template <typename T> void MinMax<T>::reset()
{
	count = 0;
	total = minVal = maxVal = 0;
}

template <typename T> void MinMax<T>::update(T value)
{
	if(count == 0) {
		minVal = value;
		maxVal = value;
	} else {
		minVal = min(minVal, value);
		maxVal = max(maxVal, value);
	}
	total += value;
	++count;
}

template <typename T> T MinMax<T>::getAverage() const
{
	return (count == 0) ? 0 : (total / count);
}

template <typename T> size_t MinMax<T>::printTo(Print& p) const
{
	auto res = p.print(title);
	res += p.print(": count=");
	res += p.print(count);
	res += p.print(", total=");
	res += p.print(total);
	res += p.print(", min=");
	res += p.print(minVal);
	res += p.print(", max=");
	res += p.print(maxVal);
	res += p.print(", average=");
	res += p.print(getAverage());
	return res;
}

} // namespace Profiling
