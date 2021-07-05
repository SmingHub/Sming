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
 * @ingroup stream
 */

class SharedMemoryStream : public IDataSourceStream
{
public:
	/** @brief Constructor for use with pre-existing buffer
	 *  @param buffer
	 *  @param capacity Size of buffer
	 */
	SharedMemoryStream(std::shared_ptr<const char>(buffer), size_t size) : buffer(buffer), capacity(size)
	{
	}

	StreamType getStreamType() const override
	{
		return eSST_Memory;
	}

	/** @brief  Get a pointer to the current position
	 *  @retval "const char*" Pointer to current cursor position within the data stream
	 */
	char* getStreamPointer() const
	{
		return (char*)(buffer.get() + readPos);
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
		int written = std::min(bufSize, available());
		memcpy(data, buffer.get() + readPos, written);

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
	std::shared_ptr<const char> buffer;
	size_t capacity;
	size_t readPos{0};
};
