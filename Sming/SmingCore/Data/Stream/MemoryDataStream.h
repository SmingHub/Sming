/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_MEMORYDATASTREAM_H_
#define _SMING_CORE_DATA_MEMORYDATASTREAM_H_

#include "DataSourceStream.h"

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
class MemoryDataStream : public ReadWriteStream {
public:
	/** @brief Memory data stream base class
    */
	MemoryDataStream()
	{}

	virtual ~MemoryDataStream()
	{
		if (_buffer)
			free(_buffer);
	}

	//Use base class documentation
	virtual StreamType getStreamType() const
	{
		return eSST_Memory;
	}

	/** @brief  Get a pointer to the current position
	 *  @retval "const char*" Pointer to current cursor position within the data stream
	 */
	const char* getStreamPointer() const
	{
		return _buffer ? _buffer + _readPos : nullptr;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	int available()
	{
		return _size - _readPos;
	}

	using ReadWriteStream::write;

	/** @brief  Write chars to end of stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to write
     *  @retval size_t Quantity of chars written to stream
     */
	virtual size_t write(const uint8_t* buffer, size_t size);

	//Use base class documentation
	virtual size_t readMemoryBlock(char* data, size_t bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	//Use base class documentation
	virtual bool isFinished()
	{
		return _readPos >= _size;
	}

	/*
	 * User may anticipate buffer size requirements to minimise reallocations.
	 */
	bool ensureCapacity(size_t required);

private:
	char* _buffer = nullptr;
	size_t _readPos = 0;
	size_t _size = 0;
	size_t _capacity = 0;
};

/**
 * @brief Memory stream that stores unlimited number of bytes.
 *
 * @note Memory is allocated on write and released when all written
 * bytes have been read out. This behaviour differs from a circular buffer
 * as the size is not fixed.
 *
 */
class EndlessMemoryStream : public ReadWriteStream {
public:
	virtual ~EndlessMemoryStream()
	{
		delete _stream;
	}

	virtual StreamType getStreamType() const
	{
		return eSST_Memory;
	}

	virtual size_t readMemoryBlock(char* data, size_t bufSize)
	{
		return _stream ? _stream->readMemoryBlock(data, bufSize) : 0;
	}

	virtual bool seek(int len);

	virtual size_t write(const uint8_t* buffer, size_t size)
	{
		if (!_stream)
			_stream = new MemoryDataStream();

		return _stream ? _stream->write(buffer, size) : 0;
	}

	/** @todo is this behaviour consistent with DataSourceStream ?
	 * It might be desirable to return true when _stream is null.
	 */
	virtual bool isFinished()
	{
		return false;
	}

private:
	MemoryDataStream* _stream = nullptr;
};

/** @} */
#endif /* _SMING_CORE_DATA_MEMORYDATASTREAM_H_ */
