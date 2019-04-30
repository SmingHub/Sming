/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ObjectQueue.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 * @author: 12 Aug 2018 - Mikee47 <mike@sillyhouse.net>
 * 	This was in 'Structures.h' and called 'SimpleConcurrentQueue'.
 *
 ****/

#pragma once

#include "WString.h"
#include "WHashMap.h"
#include "FIFO.h"

/** @brief FIFO for objects
 *  @note Objects are not owned so construction/destruction must be managed elsewhere
 */
template <typename T, int rawSize> class ObjectQueue : public FIFO<T*, rawSize>
{
public:
	virtual ~ObjectQueue()
	{
	}

	T* peek() const
	{
		return FIFO<T*, rawSize>::count() ? FIFO<T*, rawSize>::peek() : nullptr;
	}

	T* dequeue()
	{
		return FIFO<T*, rawSize>::count() ? FIFO<T*, rawSize>::dequeue() : nullptr;
	}
};

