/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Base64OutputStream.cpp
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "Base64OutputStream.h"

// Produce line breaks in output encodings
const unsigned CHARS_PER_LINE = 72;

Base64OutputStream::Base64OutputStream(IDataSourceStream* stream, size_t resultSize)
	: StreamTransformer(stream, resultSize, (resultSize / 4))
{
	base64_init_encodestate(&state, CHARS_PER_LINE);
}

size_t Base64OutputStream::transform(const uint8_t* source, size_t sourceLength, uint8_t* target, size_t)
{
	if(sourceLength == 0) {
		return base64_encode_blockend(reinterpret_cast<char*>(target), &state);
	}

	return base64_encode_block(reinterpret_cast<const char*>(source), sourceLength, reinterpret_cast<char*>(target),
							   &state);
}
