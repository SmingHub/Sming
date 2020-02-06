#pragma once

#include <Data/Stream/ReadWriteStream.h>

/*
 * All this stream does is count the number of bytes written to it,
 * which we can read by calling `available()`. The data itself is ignored.
 */
class CounterStream : public ReadWriteStream
{
public:
	size_t write(const uint8_t* buffer, size_t size)
	{
		streamSize += size;
		return size;
	}

	int available() override
	{
		return streamSize;
	}

	uint16_t readMemoryBlock(char*, int) override
	{
		return 0;
	}

	bool isFinished() override
	{
		return true;
	}

private:
	size_t streamSize = 0;
};
