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
enum class Mode {
	ReadOnly,
	Write,		///< Write but do not erase, region should be pre-erased
	BlockErase, ///< Erase blocks as required before writing
};

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
	 *
	 * @deprecated Use `mode` parameter instead of `blockErase`
	 */
	SMING_DEPRECATED PartitionStream(Partition partition, storage_size_t offset, size_t size, bool blockErase)
		: PartitionStream(partition, offset, size, blockErase ? Mode::BlockErase : Mode::ReadOnly)
	{
	}

	/**
	 * @brief Access entire partition using stream
	 * @param partition
	 * @param blockErase Set to true to erase blocks before writing
	 *
	 * If blockErase is false then partition must be pre-erased before writing.
	 *
	 * @deprecated Use `mode` parameter instead of `blockErase`
	 */
	SMING_DEPRECATED PartitionStream(Partition partition, bool blockErase)
		: PartitionStream(partition, blockErase ? Mode::BlockErase : Mode::ReadOnly)
	{
	}

	/**
	 * @brief Access part of a partition using a stream
	 * @param partition
	 * @param offset Limit access to this starting offset
	 * @param size Limit access to this number of bytes from starting offset
	 * @param mode
	 * @note When writing in Mode::BlockErase, block erasure is only performed at the
	 * start of each block. Therefore if `offset` is not a block boundary then the corresponding
	 * block will *not* be erased first.
	 */
	PartitionStream(Partition partition, storage_size_t offset, size_t size, Mode mode = Mode::ReadOnly)
		: partition(partition), startOffset(offset), size(size), mode(mode)
	{
	}

	/**
	 * @brief Access entire partition using stream
	 * @param partition
	 * @param mode
	 *
	 * If blockErase is false then partition must be pre-erased before writing.
	 */
	PartitionStream(Partition partition, Mode mode = Mode::ReadOnly)
		: partition(partition), startOffset{0}, size(partition.size()), mode(mode)
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
	Mode mode;
};

} // namespace Storage
