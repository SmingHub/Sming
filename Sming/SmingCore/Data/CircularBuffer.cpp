/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Initial code done by Ivan Grokhotkov as part of the esp8266 core for Arduino environment.
 * https://github.com/esp8266/Arduino/blob/master/cores/esp8266/cbuf.h
 *
 * Adapted for Sming by Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "CircularBuffer.h"

CircularBuffer::CircularBuffer(int size) : buffer(new char[size]), readPos(buffer), writePos(buffer), size(size)
{
}

CircularBuffer::~CircularBuffer()
{
	delete[] buffer;
}

StreamType CircularBuffer::getStreamType()
{
	return StreamType::eSST_Memory;
}

uint16_t CircularBuffer::readMemoryBlock(char* data, int bufSize)
{
	size_t bytesAvailable = available();
	size_t sizeToRead = (bufSize < bytesAvailable) ? bufSize : bytesAvailable;
	size_t sizeRead = sizeToRead;
	char* start = readPos;
	if(writePos < readPos && sizeToRead > (size_t)((buffer + size) - readPos)) {
		size_t topSize = (buffer + size) - readPos;
		memcpy(data, readPos, topSize);
		start = buffer;
		sizeToRead -= topSize;
		data += topSize;
	}
	memcpy(data, start, sizeToRead);
	return sizeRead;
}

bool CircularBuffer::seek(int len)
{
	if(len > available()) {
		flush();
		return false;
	}

	if(readPos < writePos) {
		readPos += len;
	} else if(readPos + len > buffer + size) {
		readPos = buffer + (len - (buffer + size - readPos));
	} else {
		readPos += len;
	}

	return true;
}

bool CircularBuffer::isFinished()
{
	return (available() < 1);
}

int CircularBuffer::available()
{
	if(writePos >= readPos) {
		return writePos - readPos;
	}
	return size - (readPos - writePos);
}

size_t CircularBuffer::room() const
{
	if(writePos >= readPos) {
		return size - (writePos - readPos) - 1;
	}
	return readPos - writePos - 1;
}

String CircularBuffer::id()
{
	// TODO: check if that is printing the address of the buffer...
	return String((char*)&buffer);
}

size_t CircularBuffer::write(uint8_t charToWrite)
{
	if(!room()) {
		return 0;
	}

	*writePos = charToWrite;
	writePos = wrap(writePos + 1);

	return 1;
}

size_t CircularBuffer::write(const uint8_t* data, size_t bufSize)
{
	size_t space = room();
	size_t sizeToWrite = (bufSize < space) ? bufSize : space;
	size_t sizeWritten = sizeToWrite;
	if(writePos >= readPos && sizeToWrite > (size_t)(buffer + size - writePos)) {
		size_t topSize = buffer + size - writePos;
		memcpy(writePos, data, topSize);
		writePos = buffer;
		sizeToWrite -= topSize;
		data += topSize;
	}
	memcpy(writePos, data, sizeToWrite);
	writePos = wrap(writePos + sizeToWrite);
	return sizeWritten;
}
