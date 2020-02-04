/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * LimitedMemoryStream.h
 *
 ****/

#pragma once

#include "ReadWriteStream.h"

/**
 * @brief Memory stream that stores limited number of bytes
 *
 * Once the limit is reached the stream will discard incoming bytes on write
 *
 * @ingroup stream
 */
class LimitedMemoryStream : public ReadWriteStream
{
public:
	LimitedMemoryStream(size_t length) : buffer(new uint8_t[length]), length(length)
	{
	}

	~LimitedMemoryStream()
	{
		delete[] buffer;
	}

	StreamType getStreamType() const override
	{
		return eSST_Memory;
	}

	int available() override
	{
		return writePos - readPos;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	bool seek(int len) override;

	size_t write(const uint8_t* buffer, size_t size) override;

	bool isFinished() override
	{
		return (readPos >= length);
	}

private:
	uint8_t* buffer = nullptr;
	size_t writePos = 0;
	size_t readPos = 0;
	size_t length = 0;
};
