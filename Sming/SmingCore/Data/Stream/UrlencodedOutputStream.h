/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_DATA_URL_ENCODDED_OUTPUT_STREAM_H_
#define _SMING_CORE_DATA_URL_ENCODDED_OUTPUT_STREAM_H_

#include "MemoryDataStream.h"
#include "WHashMap.h"

/**
 * @brief      UrlEncoded Stream
 * @ingroup    stream data
 *
 *  @{
*/

typedef HashMap<String, String> HttpParams;

class UrlencodedOutputStream : public ReadWriteStream
{
public:
	/**
	 * @brief Represents key-value pairs as urlencoded string
	 * @param const HttpParams& params - the key-value parameters
	 */
	UrlencodedOutputStream(const HttpParams& params);

	virtual ~UrlencodedOutputStream()
	{
	}

	//Use base class documentation
	virtual StreamType getStreamType()
	{
		return stream.getStreamType();
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available()
	{
		return stream.available();
	}

	/** @brief  Write a single char to stream
	 *  @param  charToWrite Char to write to the stream
	 *  @retval size_t Quantity of chars written to stream (always 1)
	 */
	virtual size_t write(uint8_t charToWrite)
	{
		return 0;
	}

	/** @brief  Write chars to stream
	 *  @param  buffer Pointer to buffer to write to the stream
	 *  @param  size Quantity of chars to written
	 *  @retval size_t Quantity of chars written to stream
	 */
	virtual size_t write(const uint8_t* buffer, size_t size)
	{
		return 0;
	}

	//Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize)
	{
		return stream.readMemoryBlock(data, bufSize);
	}

	//Use base class documentation
	virtual bool seek(int len)
	{
		return stream.seek(len);
	}

	//Use base class documentation
	virtual bool isFinished()
	{
		return stream.isFinished();
	}

private:
	MemoryDataStream stream;
};

/** @} */
#endif /* _SMING_CORE_DATA_URL_ENCODDED_OUTPUT_STREAM_H_ */
