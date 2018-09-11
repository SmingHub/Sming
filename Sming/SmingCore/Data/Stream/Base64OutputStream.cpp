/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "Base64OutputStream.h"

Base64OutputStream::Base64OutputStream(ReadWriteStream* stream, size_t resultSize /* = 512 */)
	: StreamTransformer(stream, nullptr, resultSize, (resultSize / 4))

{
	base64_init_encodestate(&state);

	transformCallback = std::bind(&Base64OutputStream::encode, this, std::placeholders::_1, std::placeholders::_2,
								  std::placeholders::_3, std::placeholders::_4);
}

int Base64OutputStream::encode(uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength)
{
	int count = 0;
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
