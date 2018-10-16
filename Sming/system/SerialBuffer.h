/*
 * SerialBuffer.h
 *
 *  Created on: 22 Aug 2018
 *      Author: mikee47
 */


#ifndef __SERIALBUFFER_H
#define __SERIALBUFFER_H



/** @brief FIFO buffer used for both receive and transmit data
 *  @note For receive operations, data is written via ISR and read via task
 *  	  For transmit operations, data is written via task and read via ISR
 *  Only call routines marked with __forceinline or IRAM_ATTR from an interrupt context.
 */
struct SerialBuffer
{
public:
	~SerialBuffer()
	{
		delete _buffer;
	}

	size_t size()
	{
		return _size;
	}

	/** @brief get number of bytes stored in the buffer
	 *  @retval size_t
	 */
	__forceinline size_t available()
	{
		int ret = _wpos - _rpos;
		if(ret < 0)
			ret += _size;
		return ret;
	}

	/** @brief get number of bytes of space available in this buffer
	 *  @retval size_t
	 */
	__forceinline size_t free_space()
	{
		int ret = _rpos - _wpos - 1;
		if(ret < 0)
			ret += _size;
		return ret;
	}

	__forceinline bool empty()
	{
		return _wpos == _rpos;
	}

	/** @brief see if there's anything in the buffer
	 *  @retval int first available character, or -1 if buffer's empty
	 */
	__forceinline int peek_char()
	{
		if (!_buffer || empty())
			return -1;

		return _buffer[_rpos];
	}

	/*
	 * Take a peek at the last character written into the buffer
	 */
	__forceinline int peek_last_char()
	{
		if (!_buffer || empty())
			return -1;

		return _buffer[prev_pos(_wpos)];
	}

	__forceinline int read_char()
	{
		if (!_buffer || empty())
			return -1;

		uint8_t c = _buffer[_rpos];
		_rpos = next_pos(_rpos);
		return c;
	}

	__forceinline size_t write_char(uint8_t c)
	{
		size_t nextPos = next_pos(_wpos);
		if(nextPos == _rpos)
			return 0;
		_buffer[_wpos] = c;
		_wpos = nextPos;
		return 1;
	}

	/** @brief find a character in the buffer
	 *  @param c
	 *  @retval int position relative to current read pointer, -1 if character not found
	 */
	int find(uint8_t c)
	{
		size_t offset = _rpos;
		size_t pos = 0;
		size_t avail = available();
		while(pos < avail) {
			if(_buffer[offset + pos] == c)
				return pos;

			pos++;
			if(pos + offset == _wpos)
				break;

			if(pos + offset == _size)
				offset = -pos;
		}

		return -1;
	}

	// Must be called with interrupts disabled
	size_t resize(size_t new_size)
	{
		if(_size == new_size)
			return _size;

		uint8_t* new_buf = new uint8_t[new_size];
		if(!new_buf)
			return _size;

		size_t new_wpos = 0;
		size_t avail = available();
		while(avail-- && new_wpos < new_size)
			new_buf[new_wpos++] = read_char();

		delete[] _buffer;
		_buffer = new_buf;
		_size = new_size;
		_rpos = 0;
		_wpos = new_wpos;
		return _size;
	}

	void clear()
	{
		_rpos = _wpos = 0;
	}

private:
	__forceinline size_t next_pos(size_t pos)
	{
		size_t n = pos + 1;
		return (n == _size) ? 0 : n;
	}

	__forceinline size_t prev_pos(size_t pos)
	{
		return (pos != 0 ? pos : _size) - 1;
	}

private:
	size_t _size = 0;
	size_t _rpos = 0;
	size_t _wpos = 0;
	uint8_t* _buffer = nullptr;
};


#endif //  __SERIALBUFFER_H


