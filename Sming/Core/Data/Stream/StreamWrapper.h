/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StreamWrapper.h
 *
 * @author: 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "ReadWriteStream.h"
#include <memory>

/**
 * @brief An abstract class that provides a wrapper around a stream
 * @ingroup stream
 */
class StreamWrapper : public ReadWriteStream
{
public:
	StreamWrapper(ReadWriteStream* source) : source(source)
	{
	}

	StreamType getStreamType() const override
	{
		return eSST_Wrapper;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return source ? source->readMemoryBlock(data, bufSize) : 0;
	}

	ReadWriteStream* getSource() const
	{
		return source.get();
	}

	bool isFinished() override
	{
		return source ? source->isFinished() : true;
	}

private:
	std::unique_ptr<ReadWriteStream> source;
};
