/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * LinkedObjectList.cpp
 *
 ****/

#include "LinkedObjectList.h"

bool LinkedObjectList::add(LinkedObject* object)
{
	if(object == nullptr) {
		return false;
	}

	LinkedObject* prev = nullptr;
	auto it = mHead;
	while(it != nullptr) {
		if(it == object) {
			// Already in list
			return true;
		}
		prev = it;
		it = it->mNext;
	}

	if(prev == nullptr) {
		mHead = object;
	} else {
		prev->mNext = object;
	}
	object->mNext = it;
	return true;
}

bool LinkedObjectList::remove(LinkedObject* object)
{
	if(object == nullptr || mHead == nullptr) {
		return false;
	}

	if(mHead == object) {
		mHead = object->mNext;
		return true;
	}

	auto it = mHead;
	while(it->mNext != nullptr) {
		if(it->mNext == object) {
			it->mNext = object->mNext;
			object->mNext = nullptr;
			return true;
		}
		it = it->mNext;
	}

	return false;
}
