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
	using CheckerCallback = Delegate<bool(const HttpHeaders& headers, ReadWriteStream* source, const String& fileName)>;

	/**
	 * @param callback
	 * @param stream - The actual stream doing the work. The stream is owned and will be deleted here
	 */
	PartCheckerStream(CheckerCallback callback, ReadWriteStream* source) : StreamWrapper(source), callback(callback)
	{
	}

	bool checkHeaders(const HttpHeaders& headers)
	{
		String headerValue = headers[HTTP_HEADER_CONTENT_DISPOSITION];
		String fileName;
		// Content-Disposition: form-data; name="image"; filename=".gitignore"
		int startPos = headerValue.indexOf(FS("filename="));
		if(startPos >= 0) {
			startPos += 10; // filename="
			int endPos = headerValue.indexOf('"', startPos);
			fileName = headerValue.substring(startPos, endPos);
		}

		return callback(headers, source, fileName);
	}

	StreamType getStreamType() const override
	{
		return eSST_HeaderChecker;
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
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
	CheckerCallback callback;
};
