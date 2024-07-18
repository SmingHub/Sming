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
#include <debug_progmem.h>

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
	return int(readPos);
}

size_t PartitionStream::write(const uint8_t* data, size_t length)
{
	if(mode < Mode::Write) {
		return 0;
	}

	auto len = std::min(size_t(size - writePos), length);
	if(len == 0) {
		return 0;
	}

	if(mode == Mode::BlockErase) {
		auto endPos = writePos + len;
		if(endPos > erasePos) {
			size_t blockSize = partition.getBlockSize();
			size_t eraseLen = endPos - erasePos + blockSize - 1;
			eraseLen -= eraseLen % blockSize;
			debug_d("[PS] erase(0x%08x, 0x%08x", startOffset + erasePos, eraseLen);
			if(!partition.erase_range(startOffset + erasePos, eraseLen)) {
				return 0;
			}
			erasePos += eraseLen;
		}
	}

	if(!partition.write(startOffset + writePos, data, len)) {
		return 0;
	}

	writePos += len;
	return len;
}

} // namespace Storage
