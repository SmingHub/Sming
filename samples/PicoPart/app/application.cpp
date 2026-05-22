/**	Basic_DateTime sample application
	Author: Brian Walton (brian@riban.co.uk)
	Date: 2018-12-10
	Provides serial interface, accepting Unix timestamp
	Prints each type of DateTime::format option
*/
#include <SmingCore.h>
#include "pico/bootrom.h"
#include "boot/picobin.h"
#include <boot/uf2.h>
#include <esp_spi_flash.h>
#include <Data/CStringArray.h>

#define PARTITION_LOCATION_AND_FLAGS_SIZE 2
#define PARTITION_ID_SIZE 2
#define PARTITION_NAME_MAX 127 // name length is indicated by 7 bits
#define PARTITION_EXTRA_FAMILY_ID_MAX 3

namespace
{
struct Header {
	uint32_t first_sector : 13;
	uint32_t last_sector : 13;
	uint32_t permission_s_r : 1;
	uint32_t permission_s_w : 1;
	uint32_t permission_ns_r : 1;
	uint32_t permission_ns_w : 1;
	uint32_t permission_nsboot_r : 1;
	uint32_t permission_nsboot_w : 1;

	uint32_t has_id : 1;									  // 0
	uint32_t link_type : 2;									  // 1, 2
	uint32_t link_value : 4;								  // 3 - 6
	uint32_t accepts_num_extra_families : 2;				  // 7, 8
	uint32_t not_bootable_arm : 1;							  // 9
	uint32_t not_bootable_riscv : 1;						  // 10
	uint32_t uf2_download_ab_non_bootable_owner_affinity : 1; // 11
	uint32_t has_name : 1;									  // 12
	uint32_t uf2_download_no_reboot : 1;					  // 13
	uint32_t accepts_default_family_rp2040 : 1;				  // 14
	uint32_t accepts_default_family_absolute : 1;			  // 15
	uint32_t accepts_default_family_data : 1;				  // 16
	uint32_t accepts_default_family_rp2350_arm_s : 1;		  // 17
	uint32_t accepts_default_family_rp2350_riscv : 1;		  // 18
	uint32_t accepts_default_family_rp2350_arm_ns : 1;		  // 19

	uint32_t startOffset() const
	{
		return first_sector * INTERNAL_FLASH_SECTOR_SIZE;
	}

	uint32_t endOffset() const
	{
		return (last_sector + 1) * INTERNAL_FLASH_SECTOR_SIZE;
	}
};

/*
 * Stores partition table information and data read status
 */
struct PicoPartitionTable {
	uint32_t fields;
	uint32 partition_count : 8;
	uint32_t table_present : 1;
	Header unpartitioned_space;
	uint32_t partition_data[PARTITION_TABLE_MAX_PARTITIONS * (PARTITION_LOCATION_AND_FLAGS_SIZE + PARTITION_ID_SIZE)];
};

struct PicoPartitionTableIterator {
	const PicoPartitionTable& table;
	uint16_t pos;
	uint8_t current_partition;
	int status;
};

/*
 * Stores information on each partition
 */
struct PicoPartition {
	Header header;
	uint64_t partition_id;
	char name[PARTITION_NAME_MAX + 1];
	uint32_t extra_family_id_count;
	uint32_t extra_family_ids[PARTITION_EXTRA_FAMILY_ID_MAX];
};

/*
 * Read the partition table information.
 *
 * See the RP2350 datasheet 5.1.2, 5.4.8.16 for flags and structures that can be specified.
 */
bool readPartitionTable(PicoPartitionTable& pt)
{
	// Reads fixed size fields
	uint32_t flags = PT_INFO_PT_INFO | PT_INFO_PARTITION_LOCATION_AND_FLAGS | PT_INFO_PARTITION_ID;
	int rc = rom_get_partition_table_info(&pt.fields, sizeof(pt), flags);
	if(rc < 0) {
		panic("rom_get_partition_table_info returned %d", rc);
		pt = PicoPartitionTable{};
		return false;
	}

	assert(pt.fields == flags);

	return true;
}

/*
 * Extract each partition information
 */
bool readNextPartition(PicoPartitionTableIterator& it, PicoPartition& p)
{
	auto& pt = it.table;
	if(it.current_partition >= pt.partition_count) {
		return false;
	}

	size_t pos = it.pos;
	p = PicoPartition{};
	memcpy(&p.header, &pt.partition_data[pos], sizeof(p.header));
	pos += 2;

	if(p.header.has_id) {
		uint32_t id_low = pt.partition_data[pos++];
		uint32_t id_high = pt.partition_data[pos++];
		p.partition_id = (uint64_t(id_high) << 32) | id_low;
	}
	it.pos = pos;

	if(p.header.accepts_num_extra_families || p.header.has_name) {
		// Read variable length fields
		uint32_t extra_family_ids_and_name[PARTITION_EXTRA_FAMILY_ID_MAX +
										   (((PARTITION_NAME_MAX + 1) / sizeof(uint32_t)) + 1)];
		uint32_t flags = PT_INFO_SINGLE_PARTITION | PT_INFO_PARTITION_FAMILY_IDS | PT_INFO_PARTITION_NAME;
		int rc = rom_get_partition_table_info(extra_family_ids_and_name, sizeof(extra_family_ids_and_name),
											  (it.current_partition << 24 | flags));
		if(rc < 0) {
			panic("rom_get_partition_table_info returned %d", rc);
			return false;
		}
		pos = 0;
		[[maybe_unused]] uint32_t fields = extra_family_ids_and_name[pos++];
		assert(fields == flags);
		for(size_t i = 0; i < p.extra_family_id_count; i++, pos++) {
			p.extra_family_ids[i] = extra_family_ids_and_name[pos];
		}

		if(p.header.has_name) {
			auto name_buf = reinterpret_cast<const uint8_t*>(&extra_family_ids_and_name[pos]);
			uint8_t name_length = *name_buf++ & 0x7F;
			memcpy(p.name, name_buf, name_length);
			p.name[name_length] = '\0';
		}
	}

	it.current_partition++;
	return true;
}

CStringArray parseUf2Flags(const Header& hdr)
{
	CStringArray ids;
	if(hdr.accepts_default_family_absolute) {
		ids.add("absolute");
	}
	if(hdr.accepts_default_family_rp2040) {
		ids.add("rp2040");
	}
	if(hdr.accepts_default_family_rp2350_arm_s) {
		ids.add("rp2350-arm-s");
	}
	if(hdr.accepts_default_family_rp2350_arm_ns) {
		ids.add("rp2350-arm-ns");
	}
	if(hdr.accepts_default_family_rp2350_riscv) {
		ids.add("rp2350-riscv");
	}
	if(hdr.accepts_default_family_data) {
		ids.add("data");
	}
	return ids;
}

String parseUf2Extra(uint32_t family_id)
{
	switch(family_id) {
	case CYW43_FIRMWARE_FAMILY_ID:
		return F("cyw43-firmware");
	default:
		return String(family_id, HEX, 8);
	}
}

int printPartitionInfo()
{
	PicoPartitionTable pt;
	readPartitionTable(pt);
	if(!pt.table_present) {
		printf("there is no partition table\n");
	} else if(pt.partition_count == 0) {
		printf("the partition table is empty\n");
	}

	auto printPermissions = [](const Header& hdr) {
		Serial << " S(" << (hdr.permission_s_r ? "r" : "") << (hdr.permission_s_w ? "w" : "") << ") NSBOOT("
			   << (hdr.permission_nsboot_r ? "r" : "") << (hdr.permission_nsboot_w ? "w" : "") << ") NS("
			   << (hdr.permission_ns_r ? "r" : "") << (hdr.permission_ns_w ? "w" : "") << ")";
	};

	CStringArray familyIds = parseUf2Flags(pt.unpartitioned_space);
	Serial << "un-partitioned_space:";
	printPermissions(pt.unpartitioned_space);
	Serial << " uf2 { " << familyIds.join(", ") << " }" << endl;

	if(pt.partition_count == 0) {
		return 0;
	}
	Serial << "partitions:" << endl;
	PicoPartition p;
	PicoPartitionTableIterator it{pt};
	while(readNextPartition(it, p)) {
		Serial << String(it.current_partition - 1, DEC, 3) << ":"
			   << "    " << String(p.header.startOffset(), HEX, 8) << " -> " << String(p.header.endOffset(), HEX, 8);
		printPermissions(p.header);
		if(p.header.has_id) {
			Serial << ", id=" << String(p.partition_id, HEX, 16);
		}
		if(p.header.has_name) {
			Serial << ", \"" << p.name << "\"";
		}

		// print UF2 family ID
		familyIds = parseUf2Flags(p.header);
		for(size_t i = 0; i < p.extra_family_id_count; i++) {
			familyIds.add(parseUf2Extra(p.extra_family_ids[i]));
		}
		Serial << ", uf2 { " << familyIds.join(", ") << " }";

		Serial << endl;
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
