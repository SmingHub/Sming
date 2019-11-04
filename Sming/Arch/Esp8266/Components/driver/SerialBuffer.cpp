/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SerialBuffer.cpp
 *
 * @author 22 Aug 2018 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "SerialBuffer.h"

/** @brief find a character in the buffer
 *  @param c
 *  @retval int position relative to current read pointer, -1 if character not found
 */
int SerialBuffer::find(uint8_t c)
{
	size_t offset = readPos;
	size_t pos = 0;
	size_t avail = available();
	while(pos < avail) {
		if(buffer[offset + pos] == c) {
			return pos;
		}

		pos++;
		if(pos + offset == writePos) {
			break;
		}

		if(pos + offset == size) {
			offset = -pos;
		}
	}

	return -1;
}

// Must be called with interrupts disabled
size_t SerialBuffer::resize(size_t newSize)
{
	if(size == newSize) {
		return size;
	}

	auto new_buf = new char[newSize];
	if(new_buf == nullptr) {
		return size;
	}

	size_t new_wpos = 0;
	size_t avail = available();
	while(avail-- && new_wpos < newSize) {
		new_buf[new_wpos++] = readChar();
	}

	delete[] buffer;
	buffer = new_buf;
	size = newSize;
	readPos = 0;
	writePos = new_wpos;
	return size;
}
