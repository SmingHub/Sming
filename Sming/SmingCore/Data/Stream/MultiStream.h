/*
 * MultiStream.h
 *
 *  Created on: Nov 7, 2018
 *      Author: slavey
 */

#ifndef SMINGCORE_DATA_STREAM_MULTISTREAM_H_
#define SMINGCORE_DATA_STREAM_MULTISTREAM_H_

#include "ReadWriteStream.h"

class MultiStream: public ReadWriteStream
{
public:
	virtual ~MultiStream();

	virtual StreamType getStreamType() const
	{
		return eSST_Unknown;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	virtual int available()
	{
		return -1;
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
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	//Use base class documentation
	virtual bool isFinished();

protected:
	virtual ReadWriteStream* getNextStream() = 0;

	virtual bool onCompleted()
	{
		return false;
	}

	virtual void onNextStream()
	{
		stream = nextStream;
		nextStream = nullptr;
	}

protected:
	ReadWriteStream* stream = nullptr;
	ReadWriteStream* nextStream = nullptr;

	bool finished = false;
};

#endif /* SMINGCORE_DATA_STREAM_MULTISTREAM_H_ */
