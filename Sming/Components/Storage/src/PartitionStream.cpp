/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PartitionStream.cpp
 *
 ****/

#include "include/Storage/PartitionStream.h"

namespace Storage
{
uint16_t PartitionStream::readMemoryBlock(char* data, int bufSize)
{
	int len = std::min(bufSize, available());
	return partition.read(startOffset + readPos, data, len) ? len : 0;
}

int PartitionStream::seekFrom(int offset, SeekOrigin origin)
{
	size_t newPos;
	switch(origin) {
	case SeekOrigin::Start:
		newPos = offset;
		break;
	case SeekOrigin::Current:
		newPos = readPos + offset;
		break;
	case SeekOrigin::End:
		newPos = size + offset;
		break;
	default:
		return -1;
	}

	if(newPos > size) {
		return -1;
	}

	readPos = newPos;
	return readPos;
}

size_t PartitionStream::write(const uint8_t* data, size_t length)
{
	auto len = std::min(size - writePos, length);
	if(len != 0) {
		if(!partition.write(startOffset + writePos, data, len)) {
			len = 0;
		} else {
			writePos += len;
		}
	}

	// Return amount actually written
	return len;
}

} // namespace Storage
