/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FlashMemoryStream.h
 *
 * @author: 23 Oct 2018 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "DataSourceStream.h"

/** @addtogroup stream
 *  @{
 */

/*
 * FlashMemoryDataStream
 *
 * Provides a stream buffer on flash storage, so it's read-only
 *
 */
class FlashMemoryStream : public IDataSourceStream
{
public:
	FlashMemoryStream(const FlashString& flashString) : flashString(flashString)
	{
	}

	StreamType getStreamType() const override
	{
		return eSST_Memory;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	int available() override
	{
		return flashString.length() - readPos;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	bool seek(int len) override;

	bool isFinished() override
	{
		return readPos >= flashString.length();
	}

private:
	const FlashString& flashString;
	size_t readPos = 0;
};

/** @} */
