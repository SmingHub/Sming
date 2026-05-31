#ifndef SOC_RP2040

#include "include/pico_partition.h"
#include <pico/bootrom.h>

namespace Pico
{
bool getPartitionTableInfo(PartitionTableInfo& info)
{
	int rc = rom_get_partition_table_info(&info.fields, sizeof(info), PT_INFO_PT_INFO);
	return rc >= 0;
}

bool getPartitionInfo(PartitionInfo& info, uint8_t partIndex)
{
	info = Pico::PartitionInfo{};
	uint32_t buffer[sizeof(info) / sizeof(uint32_t)];

	uint32_t flags = PT_INFO_SINGLE_PARTITION | PT_INFO_PARTITION_LOCATION_AND_FLAGS | PT_INFO_PARTITION_ID |
					 PT_INFO_PARTITION_FAMILY_IDS | PT_INFO_PARTITION_NAME;
	int rc = rom_get_partition_table_info(buffer, sizeof(buffer), (partIndex << 24) | flags);
	// If there's no partition table, returns 1 so check for a sensible minimum
	if(rc < 3) {
		return false;
	}

	// Copy over fixed header portion
	size_t pos{offsetof(Pico::PartitionInfo, id) / sizeof(uint32_t)};
	memcpy(&info, buffer, pos * sizeof(uint32_t));

	if(info.header.has_id) {
		info.id.low = buffer[pos++];
		info.id.high = buffer[pos++];
	}

	if(info.header.accepts_num_extra_families) {
		auto num_extra_families = info.header.accepts_num_extra_families;
		std::copy_n(&buffer[pos], num_extra_families, info.extra_family_ids);
		pos += num_extra_families;
	}

	if(info.header.has_name) {
		memcpy(&info.name_buffer, &buffer[pos], sizeof(info.name_buffer));
	}

	return true;
}

} // namespace Pico

#endif
