/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FlashMemoryStream.cpp
 *
 * @author: 23 Oct 2018 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "FlashMemoryStream.h"
#include "FlashString.h"

uint16_t FlashMemoryStream::readMemoryBlock(char* data, int bufSize)
{
	int count = std::min(available(), bufSize);
	memcpy_P(data, &flashString.flashData[readPos], count);
	return count;
}

bool FlashMemoryStream::seek(int len)
{
	size_t newPos = static_cast<size_t>(readPos + len);
	if(newPos <= flashString.length()) {
		readPos = newPos;
		return true;
	} else {
		return false;
	}
}
