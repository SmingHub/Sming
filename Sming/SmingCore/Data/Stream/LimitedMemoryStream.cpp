/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "LimitedMemoryStream.h"

LimitedMemoryStream::LimitedMemoryStream(size_t length)
{
	buffer = new uint8_t[length];
	this->length = length;
}

LimitedMemoryStream::~LimitedMemoryStream()
{
	delete[] buffer;
}

StreamType LimitedMemoryStream::getStreamType()
{
	return eSST_Memory;
}

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

size_t LimitedMemoryStream::write(uint8_t charToWrite)
{
	return write(&charToWrite, 1);
}

size_t LimitedMemoryStream::write(const uint8_t* data, size_t size)
{
	if(writePos + size <= length) {
		memcpy(buffer + writePos, data, size);
		writePos += size;
	}

	return size;
}

bool LimitedMemoryStream::isFinished()
{
	return (readPos >= length);
}
