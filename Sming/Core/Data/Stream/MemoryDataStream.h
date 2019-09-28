/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MemoryDataStream.h
 *
 ****/

#pragma once

#include "ReadWriteStream.h"

/** @addtogroup stream
 *  @{
 */

/*
 * MemoryDataStream
 *
 * This is intended to allow data to be streamed into it, then streamed back out at a later
 * date.
 *
 * It is _not_ intended to have data continuously written in and read out; memory is not reclaimed
 * as it is read.
 */
class MemoryDataStream : public ReadWriteStream
{
public:
	~MemoryDataStream()
	{
		free(buffer);
	}

	StreamType getStreamType() const override
	{
		return eSST_Memory;
	}

	/** @brief  Get a pointer to the current position
	 *  @retval "const char*" Pointer to current cursor position within the data stream
	 */
	const char* getStreamPointer() const
	{
		return buffer ? buffer + readPos : nullptr;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	int available() override
	{
		return size - readPos;
	}

	using ReadWriteStream::write;

	/** @brief  Write chars to end of stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to write
     *  @retval size_t Quantity of chars written to stream
     */
	size_t write(const uint8_t* buffer, size_t size) override;

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	int seekFrom(int offset, unsigned origin) override;

	bool isFinished() override
	{
		return readPos >= size;
	}

	/*
	 * @brief Pre-allocate stream to given size
	 * @param minCapacity Total minimum number of bytes required in stream
	 * @retval bool true on success
	 * @note Memory is only reallocated if smaller than requested size.
	 * If reallocation fails the existing stream content is preserved.
	 * Intended use is to reduce or eliminate buffer reallocations if the
	 * size is known in advance. Provided subsequent write operations do
	 * not exceed the total capacity they are guaranteed to succeed, so return
	 * value checking may be skipped.
	 */
	bool ensureCapacity(size_t minCapacity);

private:
	char* buffer = nullptr; ///< Stream content stored here
	size_t readPos = 0;		///< Offset to current read position
	size_t size = 0;		///< Number of bytes stored in stream (i.e. the write position)
	size_t capacity = 0;	///< Number of bytes allocated in buffer
};

/** @} */
