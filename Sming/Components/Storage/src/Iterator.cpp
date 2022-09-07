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

bool Iterator::next()
{
	while(mDevice != nullptr) {
		while(uint8_t(++mPos) < mDevice->partitions().count()) {
			auto entry = mDevice->partitions()[mPos];

			if(mSearch.type != Partition::Type::any && mSearch.type != entry.type()) {
				continue;
			}

			if(mSearch.subType != Partition::SubType::any && mSearch.subType != entry.subType()) {
				continue;
			}

			return true;
		}

		if(mSearch.device != nullptr) {
			break;
		}

		mDevice = mDevice->getNext();
		mPos = beforeStart;
	}

	mDevice = nullptr;
	mPos = afterEnd;
	return false;
}

Partition Iterator::operator*() const
{
	return mDevice ? mDevice->partitions()[mPos] : Partition{};
}

} // namespace Storage
