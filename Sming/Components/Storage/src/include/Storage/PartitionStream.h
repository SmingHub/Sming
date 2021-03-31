/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PartitionStream.h
 *
 ****/

#pragma once

#include <Data/Stream/ReadWriteStream.h>
#include "Partition.h"

namespace Storage
{
/**
 * @brief Stream operating directory on a Storage partition
 *
 * To support write operations, the target region must be erased first.
 *
 * @ingroup stream
 */
class PartitionStream : public ReadWriteStream
{
public:
	PartitionStream(Partition partition, uint32_t offset, size_t size)
		: partition(partition), startOffset(offset), size(size)
	{
	}

	PartitionStream(Partition partition) : partition(partition), startOffset(0), size(partition.size())
	{
	}

	int available() override
	{
		return size - readPos;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	int seekFrom(int offset, SeekOrigin origin) override;

	size_t write(const uint8_t* buffer, size_t size) override;

	bool isFinished() override
	{
		return available() <= 0;
	}

private:
	Partition partition;
	uint32_t startOffset;
	size_t size;
	uint32_t writePos{0};
	uint32_t readPos{0};
};

} // namespace Storage
