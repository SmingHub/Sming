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

#include "DataSourceStream.h"

class StreamWrapper : public ReadWriteStream
{
public:
	StreamWrapper(ReadWriteStream* source) : source(source)
	{
	}

	virtual ~StreamWrapper()
	{
		delete source;
	}

	StreamType getStreamType() const override
	{
		return eSST_Wrapper;
	}

	ReadWriteStream* getSource()
	{
		return source;
	}

protected:
	ReadWriteStream* source;
};
