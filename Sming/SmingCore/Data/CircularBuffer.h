/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Initial code done by Ivan Grokhotkov as part of the esp8266 core for Arduino environment.
 * https://github.com/esp8266/Arduino/blob/master/cores/esp8266/cbuf.h
 *
 * Adapted for Sming by Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

/*
 * 12/8/2018 (mikee47)
 *
 *   Changed to use buffer offsets instead of multiple pointers
 */

#ifndef _SMING_CORE_DATA_CIRCULARBUFFER_H_
#define _SMING_CORE_DATA_CIRCULARBUFFER_H_

#include "Stream/DataSourceStream.h"


/**
 * @brief      Circular stream class
 * @ingroup    stream
 *
 *  @{
 */

///Base class for data source stream
class CircularBuffer: public ReadWriteStream
{
public:
	CircularBuffer(size_t size)
	{
		_buffer = new char[size];
		_size = size;
	}


	virtual ~CircularBuffer()
	{
		delete[] _buffer;
	}

	/** @brief  Get the stream type
	 *  @retval StreamType The stream type.
	 *  @todo   Return value of IDataSourceStream:getStreamType base class function should be of type StreamType, e.g. eSST_User
	 */
	virtual StreamType getStreamType() const
	{
		return StreamType::eSST_Memory;
	}

	virtual size_t readMemoryBlock(char* buffer, size_t bufSize);


	/** @brief  Move read cursor
	 *  @param  len Position within stream to move cursor to
	 *  @retval bool True on success.
	 */
	virtual bool seek(int len);

	/** @brief  Check if stream is finished
	 *  @retval bool True on success.
	 */
	virtual bool isFinished()
	{
		return available() < 1;
	}


	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	virtual int available()
	{
		int ret = _writePos - _readPos;
		if (ret < 0)
			ret += _size;
		return ret;
	}


	/**
	 * @brief Returns unique id of the resource.
	 * @retval String the unique id of the stream.
	 */
	virtual String id()
	{
		return String((uint32_t)&_buffer, HEX);
	}


	virtual size_t write(uint8_t charToWrite);


	virtual size_t write(const uint8_t *data, size_t size);


	size_t room()
	{
		int ret = _readPos - _writePos - 1;
		if (ret < 0)
			ret += _size;
		return ret;
	}


	inline void flush()
	{
		_readPos = _writePos = 0;
	}


private:
	char* _buffer = nullptr;
	size_t _size = 0;
	size_t _readPos = 0;
	size_t _writePos = 0;
};




/** @} */
#endif /* _SMING_CORE_DATA_CIRCULARBUFFER_H_ */
