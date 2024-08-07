/* $Id: Countable.h 1151 2011-06-06 21:13:05Z bhagman $
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
||
|| @description
|| | Base class for containers.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#pragma once

template <typename T> class Countable
{
public:
	Countable() = default;

	Countable(const Countable&) = delete;
	Countable(Countable&&) = delete;
	Countable& operator=(const Countable&) = delete;
	Countable& operator=(Countable&&) = delete;

	virtual ~Countable() = default;

	virtual unsigned int count() const = 0;

	virtual const T& operator[](unsigned int) const = 0;

	virtual T& operator[](unsigned int) = 0;

	const T& at(unsigned int i) const
	{
		return operator[](i);
	}
};
