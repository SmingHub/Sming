/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PartCheckerStream.h
 *
 * @author: 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Data/Stream/StreamWrapper.h>
#include <Network/Http/HttpHeaders.h>
#include <Delegate.h>

class PartCheckerStream : public StreamWrapper
{
public:
	using CheckerCallback = Delegate<bool(const HttpHeaders& headers, ReadWriteStream* source,
										  const String& elementName, const String& fileName)>;

	/**
	 * @param callback
	 * @param stream - The actual stream doing the work. The stream is owned and will be deleted here
	 */
	PartCheckerStream(CheckerCallback callback, ReadWriteStream* source) : StreamWrapper(source), callback(callback)
	{
	}

	bool checkHeaders(const HttpHeaders& headers, const String& elementName, const String& fileName)
	{
		save = callback(headers, source, elementName, fileName);
		return save;
	}

	bool isSuccess()
	{
		return save;
	}

	StreamType getStreamType() const override
	{
		return eSST_HeaderChecker;
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
		if(!save) {
			debug_d("Discarding %d bytes", size);
			return size;
		}

		return source->write(buffer, size);
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return source->readMemoryBlock(data, bufSize);
	}

	bool isFinished() override
	{
		return source->isFinished();
	}

private:
	bool save = true;
	CheckerCallback callback;
};
