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

/**
 * @brief Base class for read-only stream which generates output from multiple source streams
 * @ingroup stream data
 */
class MultiStream : public IDataSourceStream
{
public:
	~MultiStream()
	{
		delete stream;
	}

	StreamType getStreamType() const override
	{
		return eSST_Unknown;
	}

	int available() override
	{
		return -1;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	bool seek(int len) override;

	bool isFinished() override
	{
		return finished;
	}

protected:
	/**
	 * @brief Inherited class must implement this
	 * @retval IDataSourceStream* Next stream to be read out
	 * Return nullptr if there are no more streams.
	 */
	virtual IDataSourceStream* getNextStream() = 0;

private:
	IDataSourceStream* stream = nullptr;
	bool finished = false;
};
