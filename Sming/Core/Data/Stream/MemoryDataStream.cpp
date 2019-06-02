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

size_t MemoryDataStream::write(const uint8_t* data, size_t len)
{
	//TODO: add queued buffers without full copy
	if(buf == nullptr) {
		buf = (char*)malloc(len + 1);
		if(buf == nullptr)
			return 0;
		buf[len] = '\0';
		memcpy(buf, data, len);
	} else {
		int cur = size;
		int required = cur + len + 1;
		if(required > capacity) {
			capacity = required < 256 ? required + 128 : required + 64;
			debug_d("realloc %d -> %d", size, capacity);
			char* new_buf;
			//realloc can fail, store the result in temporary pointer
			new_buf = (char*)realloc(buf, capacity);

			if(new_buf == nullptr) {
				return 0;
			}
			buf = new_buf;
		}
		buf[cur + len] = '\0';
		memcpy(buf + cur, data, len);
	}
	pos = buf;
	size += len;
	return len;
}

uint16_t MemoryDataStream::readMemoryBlock(char* data, int bufSize)
{
	int available = std::min((int)(size - (pos - buf)), bufSize);
	memcpy(data, pos, available);
	return available;
}

bool MemoryDataStream::seek(int len)
{
	if(len < 0)
		return false;

	pos += len;
	return true;
}
