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
	LimitedWriteStream(ReadWriteStream* source, size_t maxBytes) : StreamWrapper(source), maxBytes(maxBytes)
	{
	}

	bool isValid()
	{
		return writePos <= maxBytes;
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
		writePos += size;
		if(writePos > maxBytes) {
			// store in writePos the request bytes for writing
			return size;
		}

		return getSource()->write(buffer, size);
	}

private:
	const size_t maxBytes;
	size_t writePos{0};
};
