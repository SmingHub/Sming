/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_LIMITED_MEMORY_STREAM_H_
#define _SMING_CORE_DATA_LIMITED_MEMORY_STREAM_H_

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
	LimitedMemoryStream(size_t length);
	virtual ~LimitedMemoryStream();

	//Use base class documentation
	virtual StreamType getStreamType();

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	virtual int available()
	{
		return writePos - readPos;
	}

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
	uint8_t* buffer = NULL;
	size_t writePos = 0;
	size_t readPos = 0;
	size_t length = 0;
};

/** @} */
#endif /* _SMING_CORE_DATA_LIMITED_MEMORY_STREAM_H_ */
