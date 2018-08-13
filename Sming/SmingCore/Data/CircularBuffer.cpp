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

size_t CircularBuffer::readMemoryBlock(char* buffer, size_t bufSize)
{
	size_t sizeToRead = std::min(bufSize, (size_t)available());
	size_t sizeRead = sizeToRead;
	size_t start = _readPos;
	if (_writePos < _readPos) {
		size_t topSize = _size - _readPos;
		if (sizeToRead > topSize) {
			memcpy(buffer, _buffer + _readPos, topSize);
			start = 0;
			sizeToRead -= topSize;
			buffer += topSize;
		}
	}
	memcpy(buffer, _buffer + start, sizeToRead);

	return sizeRead;
}

bool CircularBuffer::seek(int len)
{
	if (len < 0 || len > available())
		return false;

	_readPos = (_readPos + len) % _size;

	return true;
}

size_t CircularBuffer::write(uint8_t charToWrite)
{
	if (room() == 0)
		return 0;

	_buffer[_writePos++] = charToWrite;
	if (_writePos == _size)
		_writePos = 0;

	return 1;
}

size_t CircularBuffer::write(const uint8_t* data, size_t size)
{
	size_t sizeToWrite = std::min(size, room());
	size_t sizeWritten = sizeToWrite;
	if (_writePos >= _readPos) {
		size_t topSize = _size - _writePos;
		if (sizeToWrite > topSize) {
			memcpy(_buffer + _writePos, data, topSize);
			_writePos = 0;
			sizeToWrite -= topSize;
			data += topSize;
		}
	}
	memcpy(_buffer + _writePos, data, sizeToWrite);
	_writePos = (_writePos + sizeToWrite) % _size;

	return sizeWritten;
}
