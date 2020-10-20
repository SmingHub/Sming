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
 * @brief Memory stream operating on fixed-size buffer
 * 		  Once the limit is reached the stream will discard incoming bytes on write
 *
 * @ingroup stream
 */
class LimitedMemoryStream : public ReadWriteStream
{
public:
	/** @brief Constructor for use with pre-existing buffer
	 *  @param buffer
	 *  @param length Size of buffer
	 *  @param available How much valid data already present in buffer
	 *  @param owned If true, buffer will be freed when this stream is destroyed
	 */
	LimitedMemoryStream(void* buffer, size_t length, size_t available, bool owned)
		: owned(owned), buffer(static_cast<char*>(buffer)), capacity(length),
		  writePos(available <= length ? available : length)
	{
	}

	/** @brief Constructor to allocate internal buffer for use
	 *  @param length Size of buffer
	 */
	LimitedMemoryStream(size_t length) : LimitedMemoryStream(new uint8_t[length], length, 0, true)
	{
	}

	~LimitedMemoryStream()
	{
		if(owned) {
			delete[] buffer;
		}
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
		return (readPos >= capacity);
	}

	bool moveString(String& s) override;

private:
	bool owned;
	char* buffer;
	size_t capacity;
	size_t writePos{0};
	size_t readPos{0};
};
