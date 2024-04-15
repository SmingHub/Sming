/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SharedMemoryStream.h
 *
 ****/

#pragma once

#include "DataSourceStream.h"
#include <memory>

/**
 * @brief Memory stream operating on fixed shared buffer.
 * 
 * One reason for templating this class is for distinction between `char` or `const char` types,
 * to avoid dangerous casts. Elements may be structures or other types.
 *
 * @ingroup stream
 */
template <typename T> class SharedMemoryStream : public IDataSourceStream
{
public:
	/** @brief Constructor for use with pre-existing buffer
	 *  @param buffer
	 *  @param size Size of buffer in elements
	 */
	SharedMemoryStream(std::shared_ptr<T>(buffer), size_t size) : buffer(buffer), capacity(size * sizeof(buffer[0]))
	{
	}

	StreamType getStreamType() const override
	{
		return eSST_Memory;
	}

	int available() override
	{
		return capacity - readPos;
	}

	size_t getCapacity() const
	{
		return capacity;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		size_t written = std::min(bufSize, available());
		auto bufptr = reinterpret_cast<const uint8_t*>(buffer.get()) + readPos;
		memcpy(data, bufptr, written);

		return written;
	}

	bool seek(int len) override
	{
		if(readPos + len > capacity) {
			return false;
		}

		readPos += len;
		return true;
	}

	bool isFinished() override
	{
		return available() <= 0;
	}

private:
	std::shared_ptr<T> buffer;
	size_t capacity;
	size_t readPos{0};
};
