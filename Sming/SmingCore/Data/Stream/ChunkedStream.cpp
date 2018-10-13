/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "ChunkedStream.h"

ChunkedStream::ChunkedStream(ReadWriteStream* stream, size_t resultSize /* = 512 */)
	: StreamTransformer(stream, nullptr, resultSize, resultSize - 12)
{
	transformCallback = std::bind(&ChunkedStream::encode, this, std::placeholders::_1, std::placeholders::_2,
								  std::placeholders::_3, std::placeholders::_4);
}

int ChunkedStream::encode(uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength)
{
	if(sourceLength == 0) {
		const char* end = "0\r\n\r\n";
		memcpy(target, end, strlen(end));
		return strlen(end);
	}

	int offset = 0;
	char chunkSize[5] = {0};
	ets_sprintf(chunkSize, "%X", sourceLength);

	memcpy(target, chunkSize, strlen(chunkSize));
	offset += strlen(chunkSize);

	// \r\n
	memcpy(target + offset, "\r\n", 2);
	offset += 2;

	memcpy(target + offset, source, sourceLength);
	offset += sourceLength;

	// \r\n
	memcpy(target + offset, "\r\n", 2);
	offset += 2;

	return offset;
}
