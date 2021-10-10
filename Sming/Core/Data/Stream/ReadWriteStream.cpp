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
	while(!source->isFinished()) {
		size_t count = source->readMemoryBlock(buffer, bufSize);
		if(count == 0) {
			continue;
		}
		auto written = write(reinterpret_cast<uint8_t*>(buffer), count);
		source->seek(written);
		total += written;
		if(written != count) {
			break;
		}
	}

	return total;
}
