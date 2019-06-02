/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ReadWriteStream.cpp
 *
 ****/

#include "ReadWriteStream.h"

static constexpr size_t maxBufferSize = 512;

size_t ReadWriteStream::copyFrom(IDataSourceStream* source, size_t size)
{
	if(source == nullptr || size == 0) {
		return 0;
	}

	auto bufSize = std::min(size, maxBufferSize);
	char buffer[bufSize];
	size_t total = 0;
	size_t count;
	while((count = source->readMemoryBlock(buffer, sizeof(buffer))) != 0) {
		write(reinterpret_cast<uint8_t*>(buffer), count);
		source->seek(count);
		total += count;
	}

	return total;
}
