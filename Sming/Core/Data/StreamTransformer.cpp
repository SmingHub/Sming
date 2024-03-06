/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StreamTransformer.cpp
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "StreamTransformer.h"

#define NETWORK_SEND_BUFFER_SIZE 1024

uint16_t StreamTransformer::readMemoryBlock(char* data, int bufSize)
{
	if(!isValid()) {
		return 0;
	}

	if(!tempStream) {
		tempStream = std::make_unique<CircularBuffer>(NETWORK_SEND_BUFFER_SIZE + 10);
	}

	// Use provided buffer as a temporary store for this operation
	fillTempStream(data, bufSize);

	return tempStream->readMemoryBlock(data, bufSize);
}

void StreamTransformer::fillTempStream(char* buffer, size_t bufSize)
{
	size_t room;
	auto maxChunkSize = std::min(bufSize, blockSize);
	while((room = tempStream->room()) >= maxChunkSize) {
		auto chunkSize = sourceStream->readMemoryBlock(buffer, maxChunkSize);
		if(chunkSize == 0) {
			break;
		}

		saveState();
		size_t outLength = transform(reinterpret_cast<const uint8_t*>(buffer), chunkSize, result.get(), resultSize);
		if(outLength > room) {
			restoreState();
			return;
		}

		auto written = tempStream->write(result.get(), outLength);
		(void)written;
		assert(written == outLength);

		sourceStream->seek(chunkSize);
	}

	if(sourceStream->isFinished()) {
		auto outLength = transform(nullptr, 0, result.get(), resultSize);
		auto written = tempStream->write(result.get(), outLength);
		(void)written;
		assert(written == outLength);
	}
}

bool StreamTransformer::seek(int len)
{
	return tempStream->seek(len);
}

bool StreamTransformer::isFinished()
{
	return (sourceStream->isFinished() && (tempStream == nullptr || tempStream->isFinished()));
}
