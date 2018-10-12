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

#ifndef _SMING_CORE_DATA_CIRCULAR_BUFFER_H_
#define _SMING_CORE_DATA_CIRCULAR_BUFFER_H_

#include "Stream/ReadWriteStream.h"

/**
 * @brief      Circular stream class
 * @ingroup    stream
 *
 *  @{
*/

///Base class for data source stream
class CircularBuffer : public ReadWriteStream
{
public:
	CircularBuffer(int size);

	virtual ~CircularBuffer();

	/** @brief  Get the stream type
     *  @retval StreamType The stream type.
     *  @todo   Return value of IDataSourceStream:getStreamType base class function should be of type StreamType, e.g. eSST_User
     */
	virtual StreamType getStreamType();

	/** @brief  Read a block of memory
     *  @param  data Pointer to the data to be read
     *  @param  bufSize Quantity of chars to read
     *  @retval uint16_t Quantity of chars read
     *  @todo   Should IDataSourceStream::readMemoryBlock return same data type as its bufSize param?
     */
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	/** @brief  Move read cursor
	 *  @param  len Position within stream to move cursor to
	 *  @retval bool True on success.
	 */
	virtual bool seek(int len);

	/** @brief  Check if stream is finished
     *  @retval bool True on success.
     */
	virtual bool isFinished();

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	virtual int available();

	/**
	 * @brief Returns unique id of the resource.
	 * @retval String the unique id of the stream.
	 */
	virtual String id();

	virtual size_t write(uint8_t charToWrite);

	/** @brief  Write chars to stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to writen
     *  @retval size_t Quantity of chars written to stream
     */
	virtual size_t write(const uint8_t* buffer, size_t size);

	size_t room() const;

	inline void flush()
	{
		readPos = buffer;
		writePos = buffer;
	}

private:
	inline char* wrap(char* ptr) const
	{
		return (ptr == buffer + size) ? buffer : ptr;
	}

private:
	char* buffer = NULL;
	char* readPos = NULL;
	char* writePos = NULL;
	int size = 0;
};

/** @} */
#endif /* _SMING_CORE_DATA_CIRCULAR_BUFFER_H_ */
