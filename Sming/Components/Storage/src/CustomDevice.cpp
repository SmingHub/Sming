/*
 * CustomDevice.cpp
 */

#include "include/Storage/CustomDevice.h"
#include <debug_progmem.h>

namespace Storage
{
namespace
{
class Partitions : public PartitionTable
{
public:
	Partition add(const Partition::Info* info)
	{
		return mEntries.add(info) ? Partition(mDevice, *info) : Partition{};
	}

	void clear()
	{
		mEntries.clear();
	}
};

} // namespace

Partition CustomDevice::createPartition(const Partition::Info* info)
{
	return static_cast<Partitions&>(mPartitions).add(info);
}

} // namespace Storage
