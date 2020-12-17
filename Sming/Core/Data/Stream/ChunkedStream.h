/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ChunkedStream.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include "../StreamTransformer.h"

/**
 * @brief Read-only stream to obtain data using HTTP chunked encoding
 *
 * Used where total length of stream is not known in advance
 *
 * @ingroup  stream data
 */
class ChunkedStream : public StreamTransformer
{
public:
	ChunkedStream(IDataSourceStream* stream, size_t resultSize = 512);

protected:
	size_t transform(const uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength) override;
};
