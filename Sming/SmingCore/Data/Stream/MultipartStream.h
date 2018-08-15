/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_DATA_MPARTSTREAM_H_
#define _SMING_CORE_DATA_MPARTSTREAM_H_

#include "DataSourceStream.h"
#include "Delegate.h"
#include "../HttpHeaders.h"

/**
 * @brief      Multipart stream class
 * @ingroup    stream data
 *
 *  @{
*/

typedef struct
{
	HttpHeaders* headers = nullptr;
	IDataSourceStream* stream = nullptr;
} HttpPartResult;

typedef Delegate<HttpPartResult()> HttpPartProducerDelegate;


class MultipartStream : public IDataSourceStream {
public:
	MultipartStream(HttpPartProducerDelegate delegate) : _producer(delegate)
	{}

	virtual ~MultipartStream()
	{
		delete _stream;
		delete _nextStream;
	}

	//Use base class documentation
	virtual StreamType getStreamType() const
	{
		// TODO: fix this...
		return _stream ? _stream->getStreamType() : eSST_Unknown;
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
	virtual size_t readMemoryBlock(char* data, size_t bufSize);

	//Use base class documentation
	virtual bool seek(int len)
	{
		return _stream ? _stream->seek(len) : false;
	}

	//Use base class documentation
	virtual bool isFinished()
	{
		return _finished && (!_stream || _stream->isFinished());
	}

	/**
	 * @brief Returns the generated boundary
	 *
	 * @retval const char*
	 */
	const char* getBoundary();

private:
	HttpPartProducerDelegate _producer = nullptr;

	IDataSourceStream* _stream = nullptr;
	IDataSourceStream* _nextStream = nullptr;

	char _boundary[16] = {0};

	bool _finished = false;
};

/** @} */

#endif /* _SMING_CORE_DATA_MPARTSTREAM_H_ */
