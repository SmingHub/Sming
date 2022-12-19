/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Iterator.cpp
 *
 ****/

#include "include/Storage/Iterator.h"
#include "include/Storage/SpiFlash.h"

namespace Storage
{
Iterator::Iterator(Partition::Type type, uint8_t subtype) : mSearch{nullptr, type, subtype}, mDevice(spiFlash)
{
	next();
}

void Iterator::next()
{
	while(mDevice != nullptr) {
		mInfo = mInfo ? mInfo->getNext() : mDevice->partitions().mEntries.head();
		if(mInfo == nullptr) {
			if(mSearch.device != nullptr) {
				break;
			}
			mDevice = mDevice->getNext();
			mInfo = nullptr;
			continue;
		}

		if(mInfo->match(mSearch.type, mSearch.subType)) {
			break;
		}
	}
}

} // namespace Storage
