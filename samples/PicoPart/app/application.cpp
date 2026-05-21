/**	Basic_DateTime sample application
	Author: Brian Walton (brian@riban.co.uk)
	Date: 2018-12-10
	Provides serial interface, accepting Unix timestamp
	Prints each type of DateTime::format option
*/
#include <SmingCore.h>
#include "pico/bootrom.h"
#include "boot/picobin.h"
#include <esp_spi_flash.h>
#include "uf2_family_ids.h"

#define PART_LOC_FIRST(x)                                                                                              \
	(((x)&PICOBIN_PARTITION_LOCATION_FIRST_SECTOR_BITS) >> PICOBIN_PARTITION_LOCATION_FIRST_SECTOR_LSB)
#define PART_LOC_LAST(x)                                                                                               \
	(((x)&PICOBIN_PARTITION_LOCATION_LAST_SECTOR_BITS) >> PICOBIN_PARTITION_LOCATION_LAST_SECTOR_LSB)

#define PARTITION_LOCATION_AND_FLAGS_SIZE 2
#define PARTITION_ID_SIZE 2
#define PARTITION_NAME_MAX 127 // name length is indicated by 7 bits
#define PARTITION_TABLE_FIXED_INFO_SIZE                                                                                \
	(4 + PARTITION_TABLE_MAX_PARTITIONS * (PARTITION_LOCATION_AND_FLAGS_SIZE + PARTITION_ID_SIZE))

namespace
{
/*
 * Stores partition table information and data read status
 */
struct PicoPartitionTable {
	uint32_t table[PARTITION_TABLE_FIXED_INFO_SIZE];
	uint32_t fields;
	bool has_partition_table;
	int partition_count;
	uint32_t unpartitioned_space_first_sector;
	uint32_t unpartitioned_space_last_sector;
	uint32_t flags_and_permissions;
	int current_partition;
	size_t pos;
	int status;
};

/*
 * Stores information on each partition
 */
struct PicoPartition {
	uint32_t first_sector;
	uint32_t last_sector;
	uint32_t flags_and_permissions;
	bool has_id;
	uint64_t partition_id;
	bool has_name;
	char name[PARTITION_NAME_MAX + 1];
	uint32_t extra_family_id_count;
	uint32_t extra_family_ids[PARTITION_EXTRA_FAMILY_ID_MAX];
};

/*
 * Read the partition table information.
 *
 * See the RP2350 datasheet 5.1.2, 5.4.8.16 for flags and structures that can be specified.
 */
int readPartitionTable(PicoPartitionTable& pt)
{
	// Reads fixed size fields
	uint32_t flags = PT_INFO_PT_INFO | PT_INFO_PARTITION_LOCATION_AND_FLAGS | PT_INFO_PARTITION_ID;
	int rc = rom_get_partition_table_info(pt.table, sizeof(pt.table), flags);
	if(rc < 0) {
		pt.partition_count = 0;
		pt.status = rc;
		return rc;
	}

	size_t pos = 0;
	pt.fields = pt.table[pos++];
	assert(pt.fields == flags);
	pt.partition_count = pt.table[pos] & 0x000000FF;
	pt.has_partition_table = pt.table[pos] & 0x00000100;
	pos++;
	uint32_t location = pt.table[pos++];
	pt.unpartitioned_space_first_sector = PART_LOC_FIRST(location);
	pt.unpartitioned_space_last_sector = PART_LOC_LAST(location);
	pt.flags_and_permissions = pt.table[pos++];
	pt.current_partition = 0;
	pt.pos = pos;
	pt.status = 0;

	return 0;
}

/*
 * Extract each partition information
 */
bool readNextPartition(PicoPartitionTable& pt, PicoPartition& p)
{
	if(pt.current_partition >= pt.partition_count) {
		return false;
	}

	size_t pos = pt.pos;
	uint32_t location = pt.table[pos++];
	p.first_sector = PART_LOC_FIRST(location);
	p.last_sector = PART_LOC_LAST(location);
	p.flags_and_permissions = pt.table[pos++];
	p.has_name = p.flags_and_permissions & PICOBIN_PARTITION_FLAGS_HAS_NAME_BITS;
	p.has_id = p.flags_and_permissions & PICOBIN_PARTITION_FLAGS_HAS_ID_BITS;

	if(p.has_id) {
		uint32_t id_low = pt.table[pos++];
		uint32_t id_high = pt.table[pos++];
		p.partition_id = ((uint64_t)id_high << 32) | id_low;
	} else {
		p.partition_id = 0;
	}
	pt.pos = pos;

	p.extra_family_id_count = (p.flags_and_permissions & PICOBIN_PARTITION_FLAGS_ACCEPTS_NUM_EXTRA_FAMILIES_BITS) >>
							  PICOBIN_PARTITION_FLAGS_ACCEPTS_NUM_EXTRA_FAMILIES_LSB;
	if(p.extra_family_id_count | p.has_name) {
		// Read variable length fields
		uint32_t extra_family_ids_and_name[PARTITION_EXTRA_FAMILY_ID_MAX +
										   (((PARTITION_NAME_MAX + 1) / sizeof(uint32_t)) + 1)];
		uint32_t flags = PT_INFO_SINGLE_PARTITION | PT_INFO_PARTITION_FAMILY_IDS | PT_INFO_PARTITION_NAME;
		int rc = rom_get_partition_table_info(extra_family_ids_and_name, sizeof(extra_family_ids_and_name),
											  (pt.current_partition << 24 | flags));
		if(rc < 0) {
			pt.status = rc;
			return false;
		}
		pos = 0;
		[[maybe_unused]] uint32_t fields = extra_family_ids_and_name[pos++];
		assert(fields == flags);
		for(size_t i = 0; i < p.extra_family_id_count; i++, pos++) {
			p.extra_family_ids[i] = extra_family_ids_and_name[pos];
		}

		if(p.has_name) {
			uint8_t* name_buf = (uint8_t*)&extra_family_ids_and_name[pos];
			uint8_t name_length = *name_buf++ & 0x7F;
			memcpy(p.name, name_buf, name_length);
			p.name[name_length] = '\0';
		}
	}
	if(!p.has_name) {
		p.name[0] = '\0';
	}

	pt.current_partition++;
	return true;
}

int printPartitionInfo()
{
	PicoPartitionTable pt;
	int rc = readPartitionTable(pt);
	if(rc != 0) {
		panic("rom_get_partition_table_info returned %d", pt.status);
	}
	if(!pt.has_partition_table) {
		printf("there is no partition table\n");
	} else if(pt.partition_count == 0) {
		printf("the partition table is empty\n");
	}

	auto printPermissions = [](uint32_t flagsAndPermissions) {
		Serial << " S(" << (flagsAndPermissions & PICOBIN_PARTITION_PERMISSION_S_R_BITS ? "r" : "")
			   << (flagsAndPermissions & PICOBIN_PARTITION_PERMISSION_S_W_BITS ? "w" : "") << ") NSBOOT("
			   << (flagsAndPermissions & PICOBIN_PARTITION_PERMISSION_NSBOOT_R_BITS ? "r" : "")
			   << (flagsAndPermissions & PICOBIN_PARTITION_PERMISSION_NSBOOT_W_BITS ? "w" : "") << ") NS("
			   << (flagsAndPermissions & PICOBIN_PARTITION_PERMISSION_NS_R_BITS ? "r" : "")
			   << (flagsAndPermissions & PICOBIN_PARTITION_PERMISSION_NS_W_BITS ? "w" : "") << ")";
	};

	CStringArray familyIds;
	UF2::Family::parseFlags(familyIds, pt.flags_and_permissions);
	Serial << "un-partitioned_space:";
	printPermissions(pt.flags_and_permissions);
	Serial << " uf2 { " << familyIds.join(", ") << " }" << endl;

	if(pt.partition_count == 0) {
		return 0;
	}
	Serial << "partitions:" << endl;
	PicoPartition p;
	while(readNextPartition(pt, p)) {
		Serial << String(pt.current_partition - 1, DEC, 3) << ":"
			   << "    " << String(p.first_sector * INTERNAL_FLASH_SECTOR_SIZE, HEX, 8) << " -> "
			   << String((p.last_sector + 1) * INTERNAL_FLASH_SECTOR_SIZE, HEX, 8);
		printPermissions(p.flags_and_permissions);
		if(p.has_id) {
			Serial << ", id=" << String(p.partition_id, HEX, 16);
		}
		if(p.has_name) {
			Serial << ", \"" << p.name << "\"";
		}

		// print UF2 family ID
		familyIds.clear();
		UF2::Family::parseFlags(familyIds, p.flags_and_permissions);
		for(size_t i = 0; i < p.extra_family_id_count; i++) {
			UF2::Family::add(familyIds, p.extra_family_ids[i]);
		}
		Serial << ", uf2 { " << familyIds.join(", ") << " }";

		Serial << endl;
	}
	if(pt.status != 0) {
		panic("rom_get_partition_table_info returned %d", pt.status);
	}

	return 0;
}

} // namespace

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(true);
	printPartitionInfo();
}
