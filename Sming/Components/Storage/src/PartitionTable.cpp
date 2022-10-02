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
	mEntries.clear();
	for(unsigned i = 0; i < count; ++i) {
		auto& e = entry[i];
		// name may not be zero-terminated
		char name[Partition::nameSize + 1];
		memcpy(name, e.name, Partition::nameSize);
		name[Partition::nameSize] = '\0';
		auto info = new Partition::Info{name, e.type, e.subtype, e.offset, e.size, e.flags};
		if(!mEntries.add(info)) {
			debug_e("Out of memory");
			delete info;
			break;
		}
	}
}

} // namespace Storage
