/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_DATA_STREAMTRANSFORMER_H_
#define _SMING_CORE_DATA_STREAMTRANSFORMER_H_

#include "CircularBuffer.h"

#undef max
#undef min
#include <functional>

/**
 * @brief      Class that can be used to transform streams of data on the fly
 * @ingroup    stream data
 *
 *  @{
 */

/**
 * @brief Callback specification for the stream transformers
 *
 * @param uint8_t* in incoming stream
 * @param int inLength incoming stream length
 * @param uint8_t* out output stream
 * @param int outLength max bytes in the output stream
 *
 * @return int number of output bytes
 */
typedef std::function<int(uint8_t* in, size_t inLength, uint8_t* out, size_t outLength)> StreamTransformerCallback;

class StreamTransformer : public ReadWriteStream
{
public:
	StreamTransformer(ReadWriteStream* stream, const StreamTransformerCallback& callback, size_t resultSize = 256,
					  size_t blockSize = 64);
	virtual ~StreamTransformer();

	//Use base class documentation
	virtual StreamType getStreamType()
	{
		return sourceStream->getStreamType();
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	int available()
	{
		return -1;
	}

	/** @brief  Write a single char to stream
	 *  @param  charToWrite Char to write to the stream
	 *  @retval size_t Quantity of chars written to stream (always 1)
	 */
	virtual size_t write(uint8_t charToWrite);

	/** @brief  Write chars to stream
	 *  @param  buffer Pointer to buffer to write to the stream
	 *  @param  size Quantity of chars to written
	 *  @retval size_t Quantity of chars written to stream
	 */
	virtual size_t write(const uint8_t* buffer, size_t size);

	//Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	//Use base class documentation
	virtual bool isFinished();

	/**
	 * @brief A method that backs up the current state
	 *
	 */
	virtual void saveState(){};

	/**
	 * @brief A method that restores the last backed up state
	 */
	virtual void restoreState(){};

protected:
	StreamTransformerCallback transformCallback = nullptr;

private:
	ReadWriteStream* sourceStream = NULL;
	CircularBuffer* tempStream = NULL;
	uint8_t* result = nullptr;
	size_t resultSize;
	size_t blockSize;
};

/** @} */
#endif /* _SMING_CORE_DATA_STREAMTRANSFORMER_H_ */
