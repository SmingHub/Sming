/* $Id: FILO.h 1151 2011-06-06 21:13:05Z bhagman $
 ||
 || @author         Alexander Brevig <abrevig@wiring.org.co>
 || @url            http://wiring.org.co/
 || @contribution   Brett Hagman <bhagman@wiring.org.co>
 ||
 || @description
 || | A simple FILO / stack class, mostly for primitive types but can be used
 || | with classes if assignment to int is allowed.
 || | This FILO is not dynamic, so be sure to choose an appropriate size for it.
 || |
 || | Wiring Common API
 || #
 ||
 || @license Please see cores/Common/License.txt.
 ||
 */

#ifndef FILO_H
#define FILO_H

#include "WiringFrameworkIncludes.h"

template <typename T, unsigned int rawSize> class FILO : public Countable<T> {
public:
	const unsigned int size; // speculative feature, in case it's needed

	FILO();

	T pop();			  // get next element
	bool push(T element); // add an element
	T peek() const;		  // get the next element without releasing it from the FILO
	void flush();		  // reset to default state

	// how many elements are currently in the FILO?
	unsigned int count() const
	{
		return _numberOfElements;
	}

private:
	volatile unsigned int _numberOfElements;
	unsigned int _nextIn;
	unsigned int _nextOut;
	T _raw[rawSize];
};

template <typename T, unsigned int rawSize> FILO<T, rawSize>::FILO() : size(rawSize)
{
	flush();
}

template <typename T, unsigned int rawSize> bool FILO<T, rawSize>::push(T element)
{
	if (count() >= rawSize) {
		return false;
	}
	_raw[_numberOfElements++] = element;
	return true;
}

template <typename T, unsigned int rawSize> T FILO<T, rawSize>::pop()
{
	if (_numberOfElements > 0) {
		return _raw[--_numberOfElements];
	}
	return _raw[0];
}

template <typename T, unsigned int rawSize> T FILO<T, rawSize>::peek() const
{
	if (_numberOfElements > 0) {
		return _raw[_numberOfElements - 1];
	}
	return _raw[0];
}

template <typename T, unsigned int rawSize> void FILO<T, rawSize>::flush()
{
	_nextIn = _nextOut = _numberOfElements = 0;
}

#endif
// FILO_H
