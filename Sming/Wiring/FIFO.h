/* $Id: FIFO.h 1151 2011-06-06 21:13:05Z bhagman $
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
||
|| @description
|| | A simple FIFO class, mostly for primitive types but can be used with
|| | classes if assignment to int is allowed.
|| | This FIFO is not dynamic, so be sure to choose an appropriate size for it.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef FIFO_H
#define FIFO_H

#include "Countable.h"

template <typename T, unsigned rawSize> class FIFO : public Countable<T>
{
public:
	const unsigned size; // speculative feature, in case it's needed

	FIFO();

	T dequeue();			 // get next element
	bool enqueue(T element); // add an element
	T peek() const;			 // get the next element without releasing it from the FIFO
	void flush();			 // reset to default state

	//how many elements are currently in the FIFO?
	unsigned int count() const override
	{
		return numberOfElements;
	}

	bool empty() const
	{
		return numberOfElements == 0;
	}

	bool full() const
	{
		return (count() >= rawSize);
	}

	const T& operator[](unsigned int index) const override
	{
		return raw[index]; /* unsafe */
	}

	T& operator[](unsigned int index) override
	{
		return raw[index]; /* unsafe */
	}

protected:
	unsigned numberOfElements;
	unsigned nextIn;
	unsigned nextOut;
	T raw[rawSize];
};

template <typename T, unsigned rawSize> FIFO<T, rawSize>::FIFO() : size(rawSize)
{
	flush();
}

template <typename T, unsigned rawSize> bool FIFO<T, rawSize>::enqueue(T element)
{
	if(full()) {
		return false;
	}
	numberOfElements++;
	raw[nextIn] = element;
	// advance to next index, wrap if needed
	if(++nextIn >= rawSize) {
		nextIn = 0;
	}
	return true;
}

template <typename T, unsigned rawSize> T FIFO<T, rawSize>::dequeue()
{
	T item;
	numberOfElements--;
	item = raw[nextOut];
	if(++nextOut >= rawSize) // advance to next index, wrap if needed
		nextOut = 0;
	return item;
}

template <typename T, unsigned rawSize> T FIFO<T, rawSize>::peek() const
{
	return raw[nextOut];
}

template <typename T, unsigned rawSize> void FIFO<T, rawSize>::flush()
{
	nextIn = nextOut = numberOfElements = 0;
}

#endif
// FIFO_H
