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

int LimitedMemoryStream::seekFrom(int offset, unsigned origin)
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
		newPos = writePos + offset;
		break;
	default:
		return -1;
	}

	if(newPos > writePos) {
		return -1;
	}

	readPos = newPos;
	return readPos;
}

size_t LimitedMemoryStream::write(const uint8_t* data, size_t size)
{
	auto len = std::min(capacity - writePos, size);
	if(len != 0) {
		memcpy(buffer + writePos, data, len);
		writePos += len;
	}

	// Any data which couldn't be written is just discarded
	return size;
}

bool LimitedMemoryStream::moveString(String& s)
{
	// If we don't own the memory buffer, this operation is unsafe
	if(!owned) {
		s = nullptr;
		return false;
	}

	// Ensure size < capacity
	size_t size{writePos};
	bool sizeOk;
	if(size < capacity) {
		++size;
		sizeOk = true;
	} else {
		sizeOk = false;
	}

	assert(s.setBuffer({buffer, capacity, size - 1}));

	owned = false;
	buffer = nullptr;
	capacity = 0;
	writePos = 0;
	readPos = 0;
	return sizeOk;
}
