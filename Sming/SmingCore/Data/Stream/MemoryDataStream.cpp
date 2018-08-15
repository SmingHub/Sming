/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "MemoryDataStream.h"

/* MemoryDataStream */

bool MemoryDataStream::ensureCapacity(size_t required)
{
	if (_capacity < required) {
		// Increase buffer capacity in anticipation of further writes
		size_t new_capacity = required + ((required < 256) ? 128 : 64);
		debug_d("MemoryDataStream::realloc %u -> %u", _capacity, new_capacity);
		// realloc can fail, store the result in temporary pointer
		auto new_buf = (char*)realloc(_buffer, new_capacity);
		if (!new_buf)
			return false;

		_buffer = new_buf;
		_capacity = new_capacity;
	}

	return true;
}

size_t MemoryDataStream::write(const uint8_t* data, size_t len)
{
	// TODO: add queued buffers without full copy

	// If reallocation fails, write as much as possible in any remaining space
	if (!ensureCapacity(_size + len + 1))
		len = _capacity - _size;

	if (len == 0)
		return 0;

	memcpy(_buffer + _size, data, len);
	_size += len;

	return len;
}

size_t MemoryDataStream::readMemoryBlock(char* data, size_t bufSize)
{
	size_t len = available();
	if (len > bufSize)
		len = bufSize;
	if (len)
		memcpy(data, _buffer + _readPos, len);
	return len;
}

bool MemoryDataStream::seek(int len)
{
	int newpos = _readPos + len;
	if (newpos < 0 || newpos > (int)_size)
		return false;
	_readPos = (size_t)newpos;
	return true;
}

/* EndlessMemoryStream */

bool EndlessMemoryStream::seek(int len)
{
	if (!_stream)
		return false;

	int res = _stream->seek(len);
	if (_stream->isFinished()) {
		delete _stream;
		_stream = nullptr;
	}

	return res;
}
