/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_ENDLESS_MEMORY_STREAM_H_
#define _SMING_CORE_ENDLESS_MEMORY_STREAM_H_

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
	virtual ~EndlessMemoryStream();

	//Use base class documentation
	virtual StreamType getStreamType();

	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	/** @brief  Write a single char to stream
	 *  @param  charToWrite Char to write to the stream
	 *  @retval size_t Quantity of chars written to stream (always 1)
	 */
	virtual size_t write(uint8_t charToWrite);

	/** @brief  Write chars to stream
	 *  @param  buffer Pointer to buffer to write to the stream
	 *  @param  size Quantity of chars to write
	 *  @retval size_t Quantity of chars written to stream
	 */
	virtual size_t write(const uint8_t* buffer, size_t size);

	virtual bool isFinished();

private:
	MemoryDataStream* stream = NULL;
};

/** @} */
#endif /* _SMING_CORE_ENDLESS_MEMORY_STREAM_H_ */
