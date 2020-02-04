/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MemoryDataStream.cpp
 *
 ****/

#include "MemoryDataStream.h"

bool MemoryDataStream::ensureCapacity(size_t minCapacity)
{
	if(capacity < minCapacity) {
		size_t newCapacity = minCapacity;
		if(capacity != 0) {
			// If expanding stream, increase buffer capacity in anticipation of further writes
			newCapacity += (minCapacity < 256) ? 128 : 64;
		}
		debug_d("MemoryDataStream::realloc %u -> %u", capacity, newCapacity);
		// realloc can fail, store the result in temporary pointer
		auto newBuffer = (char*)realloc(buffer, newCapacity);
		if(newBuffer == nullptr) {
			return false;
		}

		buffer = newBuffer;
		capacity = newCapacity;
	}

	return true;
}

size_t MemoryDataStream::write(const uint8_t* data, size_t len)
{
	if(data == nullptr || len == 0) {
		return 0;
	}

	// TODO: add queued buffers without full copy

	// If reallocation fails, write as much as possible in any remaining space
	if(!ensureCapacity(size + len)) {
		len = capacity - size;
	}

	memcpy(buffer + size, data, len);
	size += len;

	return len;
}

uint16_t MemoryDataStream::readMemoryBlock(char* data, int bufSize)
{
	size_t available = std::min(size - readPos, size_t(bufSize));
	memcpy(data, buffer + readPos, available);
	return available;
}

int MemoryDataStream::seekFrom(int offset, unsigned origin)
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
		newPos = size + offset;
		break;
	default:
		return -1;
	}

	if(newPos > size) {
		return -1;
	}

	readPos = newPos;
	return readPos;
}
