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
#include <memory>

/**
 * @brief Base class for read-only stream which generates output from multiple source streams
 * @ingroup stream data
 */
class MultiStream : public IDataSourceStream
{
public:
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
	std::unique_ptr<IDataSourceStream> stream;
	bool finished{false};
};
