/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CircularBuffer.h
 *
 * Initial code done by Ivan Grokhotkov as part of the esp8266 core for Arduino environment.
 * https://github.com/esp8266/Arduino/blob/master/cores/esp8266/cbuf.h
 *
 * Adapted for Sming by Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include "Data/Stream/ReadWriteStream.h"

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
	CircularBuffer(int size) : buffer(new char[size]), readPos(buffer), writePos(buffer), size(size)
	{
	}

	~CircularBuffer()
	{
		delete[] buffer;
	}

	/** @brief  Get the stream type
     *  @retval StreamType The stream type.
     *  @todo   Return value of IDataSourceStream:getStreamType base class function should be of type StreamType, e.g. eSST_User
     */
	StreamType getStreamType() const override
	{
		return StreamType::eSST_Memory;
	}

	/** @brief  Read a block of memory
     *  @param  data Pointer to the data to be read
     *  @param  bufSize Quantity of chars to read
     *  @retval uint16_t Quantity of chars read
     *  @todo   Should IDataSourceStream::readMemoryBlock return same data type as its bufSize param?
     */
	uint16_t readMemoryBlock(char* data, int bufSize) override;

	/** @brief  Move read cursor
	 *  @param  len Position within stream to move cursor to
	 *  @retval bool True on success.
	 */
	bool seek(int len) override;

	/** @brief  Check if stream is finished
	 *  @retval bool true on success.
	 */
	bool isFinished() override
	{
		return available() < 1;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available() override;

	/**
	 * @brief Returns unique id of the resource.
	 * @retval String the unique id of the stream.
	 */
	String id() const override
	{
		return String(reinterpret_cast<uint32_t>(&buffer), HEX);
	}

	size_t write(uint8_t charToWrite) override;

	/** @brief  Write chars to stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to writen
     *  @retval size_t Quantity of chars written to stream
     */
	size_t write(const uint8_t* data, size_t size) override;

	/** @brief Get the maximum number of bytes for which write() will succeed
		@retval size_t
	*/
	size_t room() const;

	// Stream::flush()
	void flush() override
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
	char* buffer = nullptr;
	char* readPos = nullptr;
	char* writePos = nullptr;
	int size = 0;
};

/** @} */
