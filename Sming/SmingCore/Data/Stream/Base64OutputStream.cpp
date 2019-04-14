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

size_t Base64OutputStream::transform(const uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength)
{
	size_t count = 0;
	if(sourceLength == 0) {
		count = base64_encode_blockend((char*)target, &state);
	} else {
		count = base64_encode_block((const char*)source, sourceLength, (char*)target, &state);
	}

	return count;
}

void Base64OutputStream::saveState()
{
	memcpy(&lastState, &state, sizeof(base64_encodestate));
}

void Base64OutputStream::restoreState()
{
	memcpy(&state, &lastState, sizeof(base64_encodestate));
}
