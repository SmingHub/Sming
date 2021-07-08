/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * XorOutputStream.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include <Data/Stream/DataSourceStream.h>

class XorOutputStream : public IDataSourceStream
{
public:
	/**
	 * @brief Xors original stream content with the specified mask
	 * @param stream pointer to the original stream. Will be deleted after use
	 * @param mask
	 * @param maskLenth
	 */
	XorOutputStream(IDataSourceStream* stream, uint8_t* mask, size_t maskLength)
		: stream(stream), mask(mask), maskLength(maskLength)
	{
	}

	~XorOutputStream()
	{
		delete stream;
	}

	StreamType getStreamType() const override
	{
		return eSST_Transform;
	}

	int available() override
	{
		return stream->available();
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		uint16_t max = stream->readMemoryBlock(data, bufSize);
		size_t pos = maskPos;
		for(unsigned i = 0; i < max; i++) {
			pos = pos % maskLength;
			data[i] = (data[i] ^ mask[pos]);
			pos++;
		}

		return max;
	}

	bool seek(int len) override
	{
		if(!stream->seek(len)) {
			return false;
		}

		maskPos = (maskPos + len) % maskLength;
		return true;
	}

	bool isFinished() override
	{
		return stream->isFinished();
	}

private:
	IDataSourceStream* stream;
	uint8_t* mask;
	size_t maskLength;
	size_t maskPos = 0;
};
