/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * UrlencodedOutputStream.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include "MemoryDataStream.h"
#include "Network/Http/HttpParams.h"

/**
 * @brief Represents key-value pairs as urlencoded string content
 * @ingroup stream
 */
class UrlencodedOutputStream : public IDataSourceStream
{
public:
	UrlencodedOutputStream(const HttpParams& params);

	StreamType getStreamType() const override
	{
		return stream.getStreamType();
	}

	int available() override
	{
		return stream.available();
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return stream.readMemoryBlock(data, bufSize);
	}

	bool seek(int len) override
	{
		return stream.seek(len);
	}

	bool isFinished() override
	{
		return stream.isFinished();
	}

private:
	MemoryDataStream stream;
};
