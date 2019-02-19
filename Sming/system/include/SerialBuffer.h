/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SerialBuffer.h
 *
 * @author 22 Aug 2018 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#ifndef _SYSTEM_INCLUDE_SERIAL_BUFFER_H_
#define _SYSTEM_INCLUDE_SERIAL_BUFFER_H_

/** @brief FIFO buffer used for both receive and transmit data
 *  @note For receive operations, data is written via ISR and read via task
 *  	  For transmit operations, data is written via task and read via ISR
 *  Only call routines marked with __forceinline or IRAM_ATTR from an interrupt context.
 */
struct SerialBuffer {
public:
	~SerialBuffer()
	{
		delete buffer;
	}

	size_t getSize()
	{
		return size;
	}

	/** @brief get number of bytes stored in the buffer
	 *  @retval size_t
	 */
	__forceinline size_t available()
	{
		int ret = writePos - readPos;
		if(ret < 0)
			ret += size;
		return ret;
	}

	/** @brief get number of bytes of space available in this buffer
	 *  @retval size_t
	 */
	__forceinline size_t getFreeSpace()
	{
		int ret = readPos - writePos - 1;
		if(ret < 0)
			ret += size;
		return ret;
	}

	__forceinline bool isEmpty()
	{
		return writePos == readPos;
	}

	/** @brief see if there's anything in the buffer
	 *  @retval int first available character, or -1 if buffer's empty
	 */
	__forceinline int peekChar()
	{
		if(!buffer || isEmpty())
			return -1;

		return buffer[readPos];
	}

	/*
	 * Take a peek at the last character written into the buffer
	 */
	__forceinline int peekLastChar()
	{
		if(!buffer || isEmpty())
			return -1;

		return buffer[getPrevPos(writePos)];
	}

	__forceinline int readChar()
	{
		if(!buffer || isEmpty())
			return -1;

		uint8_t c = buffer[readPos];
		readPos = getNextPos(readPos);
		return c;
	}

	__forceinline size_t writeChar(uint8_t c)
	{
		size_t nextPos = getNextPos(writePos);
		if(nextPos == readPos)
			return 0;
		buffer[writePos] = c;
		writePos = nextPos;
		return 1;
	}

	/** @brief find a character in the buffer
	 *  @param c
	 *  @retval int position relative to current read pointer, -1 if character not found
	 */
	int find(uint8_t c)
	{
		size_t offset = readPos;
		size_t pos = 0;
		size_t avail = available();
		while(pos < avail) {
			if(buffer[offset + pos] == c)
				return pos;

			pos++;
			if(pos + offset == writePos)
				break;

			if(pos + offset == size)
				offset = -pos;
		}

		return -1;
	}

	// Must be called with interrupts disabled
	size_t resize(size_t newSize)
	{
		if(size == newSize)
			return size;

		uint8_t* new_buf = new uint8_t[newSize];
		if(!new_buf)
			return size;

		size_t new_wpos = 0;
		size_t avail = available();
		while(avail-- && new_wpos < newSize)
			new_buf[new_wpos++] = readChar();

		delete[] buffer;
		buffer = new_buf;
		size = newSize;
		readPos = 0;
		writePos = new_wpos;
		return size;
	}

	void clear()
	{
		readPos = writePos = 0;
	}

private:
	/** @brief Get the offset for the position before the current one */
	__forceinline size_t getNextPos(size_t pos)
	{
		size_t n = pos + 1;
		return (n == size) ? 0 : n;
	}

	/** @brief Get the offset for the position after the current one */
	__forceinline size_t getPrevPos(size_t pos)
	{
		return (pos != 0 ? pos : size) - 1;
	}

private:
	size_t size = 0;
	size_t readPos = 0;
	size_t writePos = 0;
	uint8_t* buffer = nullptr;
};

#endif //  _SYSTEM_INCLUDE_SERIAL_BUFFER_H_
