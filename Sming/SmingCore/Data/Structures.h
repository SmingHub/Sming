/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_DATA_STRUCTURES_H_
#define _SMING_CORE_DATA_STRUCTURES_H_

#include "../../Wiring/FILO.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"

/**
 * WARNING: For the moment the name "SimpleConcurrentQueue" is very misleading.
 */
template <typename T, int rawSize> class SimpleConcurrentQueue : public FIFO<T, rawSize>
{
public:
	virtual const T& operator[](unsigned int) const
	{
	}
	virtual T& operator[](unsigned int)
	{
	}

	T peek() const
	{
		if(!FIFO<T, rawSize>::numberOfElements) {
			return NULL;
		}

		return FIFO<T, rawSize>::peek();
	}

	T dequeue()
	{
		if(!FIFO<T, rawSize>::numberOfElements) {
			return NULL;
		}

		return FIFO<T, rawSize>::dequeue();
	}
};

#endif /* _SMING_CORE_DATA_STRUCTURES_H_ */
