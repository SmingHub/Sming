/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * LimitedMemoryStream.cpp
 *
 ****/

#include "LimitedMemoryStream.h"

uint16_t LimitedMemoryStream::readMemoryBlock(char* data, int bufSize)
{
	int written = std::min(bufSize, available());
	memcpy(data, buffer + readPos, written);

	return written;
}

bool LimitedMemoryStream::seek(int len)
{
	if(readPos + len > length) {
		return false;
	}

	readPos += len;

	return true;
}

size_t LimitedMemoryStream::write(const uint8_t* data, size_t size)
{
	if(writePos + size <= length) {
		memcpy(buffer + writePos, data, size);
		writePos += size;
	}

	return size;
}
