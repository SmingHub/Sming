/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * QuotedPrintableOutputStream.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include "../StreamTransformer.h"

/**
 * @brief   Read-only stream that transforms bytes of data into quoted printable data stream
 * @ingroup stream data
*/
class QuotedPrintableOutputStream : public StreamTransformer
{
public:
	/**
	 * @param stream Source stream
	 * @param resultSize The size of the intermediate buffer, created once per object and reused multiple times
	 */
	QuotedPrintableOutputStream(IDataSourceStream* stream, size_t resultSize = 512)
		: StreamTransformer(stream, resultSize, resultSize / 2)

	{
	}

protected:
	size_t transform(const uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength) override;
};
