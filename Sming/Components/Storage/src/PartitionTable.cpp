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

#ifdef ARCH_RP2040
#include <pico_partition.h>
#endif

/*
 * Stores information on each partition
 */

namespace Storage
{
void PartitionTable::load(const esp_partition_info_t* entry, unsigned count)
{
	mEntries.clear();
	for(; count != 0; --count, ++entry) {
		// name may not be zero-terminated
		char name[Partition::nameSize + 1];
		memcpy(name, entry->name, Partition::nameSize);
		name[Partition::nameSize] = '\0';
		add(name, {entry->type, entry->subtype}, entry->offset, entry->size, entry->flags);
	}
}

bool PartitionTable::loadPico()
{
#if defined(ARCH_RP2040) && !defined(SOC_RP2040)
	Pico::PartitionTableInfo table;
	if(!Pico::getPartitionTableInfo(table)) {
		return false;
	}

	mEntries.clear();

	for(uint8_t partIndex = 0; partIndex < table.partition_count; ++partIndex) {
		Pico::PartitionInfo info{};
		if(!Pico::getPartitionInfo(info, partIndex)) {
			continue;
		}
		if(!info.header.has_id || !info.header.has_name) {
			continue;
		}
		if(info.id.tag != Pico::SMING_TAG) {
			continue;
		}

		Partition::Flags partFlags{};
		if(info.header.isReadOnly()) {
			partFlags |= Partition::Flag::readOnly;
		}
		if(info.header.isEncrypted()) {
			partFlags |= Partition::Flag::encrypted;
		}
		add(info.name(), {Partition::Type(info.id.type), info.id.subtype}, info.header.startOffset(),
			info.header.size(), partFlags);
	}

	return true;

#else
	return false;
#endif
}

} // namespace Storage
