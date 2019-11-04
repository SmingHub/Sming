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

/** @addtogroup stream
 *  @{
 */

/**
 * @brief Memory stream that stores limited number of bytes
 * 		  Once the limit is reached the stream will discard incoming bytes on write
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

	//Use base class documentation
	StreamType getStreamType() const override
	{
		return eSST_Memory;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available() override
	{
		return writePos - readPos;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	//Use base class documentation
	bool seek(int len) override;

	/** @brief  Write chars to stream
	 *  @param  buffer Pointer to buffer to write to the stream
	 *  @param  size Quantity of chars to write
	 *  @retval size_t Quantity of chars written to stream
	 */
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

/** @} */
