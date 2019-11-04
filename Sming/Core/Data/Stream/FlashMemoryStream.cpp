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

int FlashMemoryStream::seekFrom(int offset, unsigned origin)
{
	size_t newPos;
	switch(origin) {
	case SEEK_SET:
		newPos = offset;
		break;
	case SEEK_CUR:
		newPos = readPos + offset;
		break;
	case SEEK_END:
		newPos = flashString.length() + offset;
		break;
	default:
		return -1;
	}

	if(newPos > flashString.length()) {
		return -1;
	}

	readPos = newPos;
	return readPos;
}
