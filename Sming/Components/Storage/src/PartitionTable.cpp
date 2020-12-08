/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PartitionTable.cpp
 *
 ****/

#include "include/Storage/PartitionTable.h"
#include "include/Storage/partition_info.h"
#include <debug_progmem.h>

namespace Storage
{
void PartitionTable::load(const esp_partition_info_t* entry, unsigned count)
{
	if(count == 0) {
		mEntries.reset();
		mCount = count;
		return;
	}

	mCount = count;
	mEntries.reset(new Partition::Info[count]);
	for(unsigned i = 0; i < count; ++i) {
		auto& e = entry[i];
		// name may not be zero-terminated
		char name[Partition::nameSize + 1];
		memcpy(name, e.name, Partition::nameSize);
		name[Partition::nameSize] = '\0';
		mEntries.get()[i] = Partition::Info{name, e.type, e.subtype, e.offset, e.size, e.flags};
	}
}

} // namespace Storage
