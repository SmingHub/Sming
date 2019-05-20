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

#pragma once

#include <esp_systemapi.h>

/** @brief FIFO buffer used for both receive and transmit data
 *  @note For receive operations, data is written via ISR and read via task
 *  	  For transmit operations, data is written via task and read via ISR
 *  Only routines marked with __forceinline or IRAM_ATTR may be called from interrupt context.
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
		if(ret < 0) {
			ret += size;
		}
		return ret;
	}

	/** @brief get number of bytes of space available in this buffer
	 *  @retval size_t
	 */
	__forceinline size_t getFreeSpace()
	{
		if(buffer == nullptr) {
			return 0;
		}
		int ret = readPos - writePos - 1;
		if(ret < 0) {
			ret += size;
		}
		return ret;
	}

	__forceinline bool isEmpty()
	{
		return (buffer == nullptr) || (writePos == readPos);
	}

	__forceinline bool isFull()
	{
		return getFreeSpace() == 0;
	}

	/** @brief see if there's anything in the buffer
	 *  @retval int first available character, or -1 if buffer's empty
	 */
	__forceinline int peekChar()
	{
		return isEmpty() ? -1 : buffer[readPos];
	}

	/*
	 * Take a peek at the last character written into the buffer
	 */
	__forceinline int peekLastChar()
	{
		return isEmpty() ? -1 : buffer[getPrevPos(writePos)];
	}

	__forceinline int readChar()
	{
		if(isEmpty()) {
			return -1;
		}

		uint8_t c = buffer[readPos];
		readPos = getNextPos(readPos);
		return c;
	}

	__forceinline size_t writeChar(uint8_t c)
	{
		size_t nextPos = getNextPos(writePos);
		if(nextPos == readPos) {
			return 0;
		}

		buffer[writePos] = c;
		writePos = nextPos;
		return 1;
	}

	/** @brief find a character in the buffer
	 *  @param c
	 *  @retval int position relative to current read pointer, -1 if character not found
	 */
	int find(uint8_t c);

	// Must be called with interrupts disabled
	size_t resize(size_t newSize);

	void clear()
	{
		readPos = writePos = 0;
	}

	/** @brief Access data directly within buffer
	 *  @param void*& OUT: the data
	 *  @retval size_t number of chars available
	 */
	__forceinline size_t getReadData(void*& data)
	{
		data = buffer + readPos;
		auto wp = writePos; // Guard against ISR changing value
		return (wp < readPos) ? size - readPos : wp - readPos;
	}

	/** @brief Skip a number of chars starting at the given read position
	 *  @param length MUST be <= value returned from peek()
	 *  @note Provided for efficient buffer access
	 */
	__forceinline void skipRead(size_t length)
	{
		readPos += length;
		if(readPos == size) {
			readPos = 0;
		}
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
	char* buffer = nullptr;
};
