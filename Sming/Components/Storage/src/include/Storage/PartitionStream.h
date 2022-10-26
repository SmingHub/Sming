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
	/**
	 * @brief Access part of a partition using a stream
	 * @param partition
	 * @param offset Limit access to this starting offset
	 * @param size Limit access to this number of bytes from starting offset
	 * @param blockErase Set to true to erase blocks before writing
	 *
	 * If blockErase is false then region must be pre-erased before writing.
	 */
	PartitionStream(Partition partition, storage_size_t offset, size_t size, bool blockErase = false)
		: partition(partition), startOffset(offset), size(size), blockErase(blockErase)
	{
	}

	/**
	 * @brief Access entire partition using stream
	 * @param partition
	 * @param blockErase Set to true to erase blocks before writing
	 *
	 * If blockErase is false then partition must be pre-erased before writing.
	 */
	PartitionStream(Partition partition, bool blockErase = false)
		: partition(partition), startOffset{0}, size(partition.size()), blockErase(blockErase)
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
	storage_size_t startOffset;
	size_t size;
	uint32_t writePos{0};
	uint32_t readPos{0};
	uint32_t erasePos{0};
	bool blockErase;
};

} // namespace Storage
