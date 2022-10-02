/*
 * CustomDevice.h
 */

#pragma once

#include "Device.h"

namespace Storage
{
/**
 * @brief Class to support dynamic partitions
 *
 * Call `createPartition` to add partitions up to a maximum of 16 entries.
 */
class CustomDevice : public Device
{
public:
	/**
	 * @brief Add new partition using given Info
	 * @param info Must be allocated using `new`: Device will take ownership
	 * @retval Partition Reference to the partition
	 */
	Partition createPartition(const Partition::Info* info);

	Partition createPartition(const String& name, Partition::Type type, uint8_t subtype, uint32_t offset, size_t size,
							  Partition::Flags flags = 0)
	{
		return createPartition(new Partition::Info{name, type, subtype, offset, size, flags});
	}

	template <typename SubType>
	Partition createPartition(const String& name, SubType subtype, uint32_t offset, size_t size,
							  Partition::Flags flags = 0)
	{
		return createPartition(name, Partition::Type(SubType::partitionType), uint8_t(subtype), offset, size, flags);
	}
};

} // namespace Storage
