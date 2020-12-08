/*
 * CustomDevice.cpp
 */

#include "include/Storage/CustomDevice.h"
#include <debug_progmem.h>

namespace Storage
{
namespace
{
static constexpr size_t maxPartitions{16}; ///< Hard limit on partition table size

class Partitions : public PartitionTable
{
public:
	Partition add(const Partition::Info& info)
	{
		if(!mEntries) {
			mEntries.reset(new Partition::Info[maxPartitions]);
		} else
			assert(mCount < maxPartitions);

		auto i = mCount++;
		mEntries.get()[i] = info;
		return operator[](i);
	}
};

} // namespace

Partition CustomDevice::createPartition(const Partition::Info& info)
{
	if(mPartitions.count() >= maxPartitions) {
		debug_e("Partition table is full for '%s'", getName().c_str());
		return Partition{};
	}

	return reinterpret_cast<Partitions&>(mPartitions).add(info);
}

} // namespace Storage
