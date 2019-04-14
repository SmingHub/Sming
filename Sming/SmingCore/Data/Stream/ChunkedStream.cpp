/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ChunkedStream.cpp
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "ChunkedStream.h"

ChunkedStream::ChunkedStream(IDataSourceStream* stream, size_t resultSize)
	: StreamTransformer(stream, resultSize, resultSize - 12)
{
}

size_t ChunkedStream::transform(const uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength)
{
	if(sourceLength == 0) {
		memcpy(target, _F("0\r\n\r\n"), 5);
		return 5;
	}

	// Header
	unsigned offset = m_snprintf(reinterpret_cast<char*>(target), targetLength, "%X\r\n", sourceLength);

	// Content
	memcpy(target + offset, source, sourceLength);
	offset += sourceLength;

	// Footer
	memcpy(target + offset, "\r\n", 2);
	offset += 2;

	return offset;
}
