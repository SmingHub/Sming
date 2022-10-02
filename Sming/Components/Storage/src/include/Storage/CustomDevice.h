/*
 * CustomDevice.h
 */

#pragma once

#include "Device.h"

namespace Storage
{
/**
 * @brief Class to support dynamic partitions
 */
class CustomDevice : public Device
{
public:
	class CustomPartitionTable : public PartitionTable
	{
	public:
		using PartitionTable::add;
		using PartitionTable::clear;
	};

	/**
	 * @brief Provide read/write access to in-memory partition table
	 */
	CustomPartitionTable& partitions()
	{
		return static_cast<CustomPartitionTable&>(mPartitions);
	}
};

} // namespace Storage
