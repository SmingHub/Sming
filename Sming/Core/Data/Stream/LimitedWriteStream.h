/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * LimitedWriteStream.h
 *
 * @author: 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Data/Stream/StreamWrapper.h>

class LimitedWriteStream : public StreamWrapper
{
public:
	LimitedWriteStream(size_t maxBytes, ReadWriteStream* source)
		: maxBytes(maxBytes), writePos(0), StreamWrapper(source)
	{
	}

	bool isSuccess()
	{
		return !(writePos > maxBytes);
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
		if(writePos + size > maxBytes) {
			writePos += size; // store in writePos the request bytes for writing
			return size;
		}

		return source->write(buffer, size);
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return source->readMemoryBlock(data, bufSize);
	}

	bool isFinished() override
	{
		return source->isFinished();
	}

private:
	const size_t maxBytes;
	size_t writePos;
};
