/*
 * ObjectQueue.h
 *   Author: mikee47
 *
 * 12/8/2018
 *
 * 	This was in 'Structures.h' and called 'SimpleConcurrentQueue'.
 *
 *
 ****/

#ifndef __OBJECTQUEUE_H
#define __OBJECTQUEUE_H

#include "WString.h"
#include "WHashMap.h"
#include "FIFO.h"

/** @brief FIFO for objects
 *  @note We don't own these objects, freeing them is not our responsibility
 *  @todo Add parameter to allow objects to be owned, so they get cleaned up automatically
 *  Can then use this queue for multiple purposes
 */
template <typename T, int rawSize> class ObjectQueue : public FIFO<T*, rawSize> {
public:
	virtual ~ObjectQueue()
	{}

	T* peek() const
	{
		return FIFO<T*, rawSize>::count() ? FIFO<T*, rawSize>::peek() : nullptr;
	}

	T* dequeue()
	{
		return FIFO<T*, rawSize>::count() ? FIFO<T*, rawSize>::dequeue() : nullptr;
	}
};

#endif // __OBJECTQUEUE_H
