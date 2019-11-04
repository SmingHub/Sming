/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EndlessMemoryStream.h
 *
 ****/

#pragma once

#include "MemoryDataStream.h"

/** @addtogroup stream
 *  @{
 */

/**
 * @brief Memory stream that stores unlimited number of bytes.
 *
 * @note Memory is allocated on write and released when all written
 * bytes have been read out. This behaviour differs from a circular buffer
 * as the size is not fixed.
 *
 */
class EndlessMemoryStream : public ReadWriteStream
{
public:
	~EndlessMemoryStream()
	{
		delete stream;
	}

	StreamType getStreamType() const override
	{
		return eSST_Memory;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return stream ? stream->readMemoryBlock(data, bufSize) : 0;
	}

	bool seek(int len) override;

	/** @brief  Write chars to stream
	 *  @param  buffer Pointer to buffer to write to the stream
	 *  @param  size Quantity of chars to write
	 *  @retval size_t Quantity of chars written to stream
	 */
	size_t write(const uint8_t* buffer, size_t size) override;

	bool isFinished() override
	{
		return false;
	}

private:
	MemoryDataStream* stream = nullptr;
};

/** @} */
