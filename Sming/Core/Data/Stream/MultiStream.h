/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MultiStream.h
 *
 *  Created on: Nov 7, 2018
 *      Author: slavey
 *
 ****/

#pragma once

#include "DataSourceStream.h"

class MultiStream : public IDataSourceStream
{
public:
	~MultiStream()
	{
		delete stream;
		delete nextStream;
	}

	StreamType getStreamType() const override
	{
		return eSST_Unknown;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	int available() override
	{
		return -1;
	}

	//Use base class documentation
	uint16_t readMemoryBlock(char* data, int bufSize) override;

	//Use base class documentation
	bool seek(int len) override;

	//Use base class documentation
	bool isFinished() override;

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
