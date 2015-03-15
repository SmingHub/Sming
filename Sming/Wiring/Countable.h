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

#ifndef COUNTABLE_H
#define COUNTABLE_H

#include "WiringFrameworkDependencies.h"

template<typename T>
class Countable
{
  public:
	virtual unsigned int count() const = 0;
	virtual const T& operator[](unsigned int) const = 0;
	virtual T& operator[](unsigned int) = 0;
	const T& at(unsigned int i) const
    {
      return operator[](i);
    }
};

#endif
// COUNTABLE_H
