/*
 * MultiStream.h
 *
 *  Created on: Nov 7, 2018
 *      Author: slavey
 */

#ifndef SMINGCORE_DATA_STREAM_MULTISTREAM_H_
#define SMINGCORE_DATA_STREAM_MULTISTREAM_H_

#include "DataSourceStream.h"

class MultiStream : public IDataSourceStream
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

	//Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	//Use base class documentation
	virtual bool isFinished();

protected:
	virtual IDataSourceStream* getNextStream() = 0;

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
	IDataSourceStream* stream = nullptr;
	IDataSourceStream* nextStream = nullptr;

	bool finished = false;
};

#endif /* SMINGCORE_DATA_STREAM_MULTISTREAM_H_ */
