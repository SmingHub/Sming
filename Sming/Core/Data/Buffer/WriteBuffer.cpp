/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WriteBuffer.cpp
 *
 ****/

#include "WriteBuffer.h"

size_t BaseWriteBuffer::write(uint8_t c)
{
	buffer[writeOffset++] = c;
	if(writeOffset == bufferSize) {
		flush();
	}
	return 1;
}

size_t BaseWriteBuffer::write(const uint8_t* data, size_t size)
{
	size_t written{0};
	while(size != 0) {
		auto copySize = std::min(bufferSize - writeOffset, size);
		memcpy(&buffer[writeOffset], data, copySize);
		writeOffset += copySize;
		written += copySize;
		data += copySize;
		size -= copySize;
		if(writeOffset == bufferSize) {
			flush();
		}
	}
	return written;
}

void BaseWriteBuffer::flush()
{
	output.write(buffer, writeOffset);
	writeOffset = 0;
}
