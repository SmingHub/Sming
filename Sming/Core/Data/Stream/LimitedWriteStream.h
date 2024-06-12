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

/**
 * @brief A stream wrapper class that limits the number of bytes that can be written.
 *        Helpful when writing on a file system or memory should be limited to the available size of the media.
 * @ingroup stream
 */
class LimitedWriteStream : public StreamWrapper
{
public:
	LimitedWriteStream(ReadWriteStream* source, size_t maxBytes) : StreamWrapper(source), maxBytes(maxBytes)
	{
	}

	bool isValid() const override
	{
		return writePos <= maxBytes;
	}

	/**	@brief  Writes characters from a buffer to output stream
	 *
	 * @param  buffer Pointer to character buffer
	 * @param  size Quantity of characters to write
	 *
	 * @retval size_t Quantity of characters written to stream.
	 * @note Surplus characters will be discarded.
	 *
	 */
	size_t write(const uint8_t* buffer, size_t size) override
	{
		writePos += size;
		if(writePos > maxBytes) {
			return size;
		}

		return getSource()->write(buffer, size);
	}

private:
	const size_t maxBytes;
	size_t writePos{0};
};
