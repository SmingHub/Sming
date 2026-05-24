/**	Basic_DateTime sample application
	Author: Brian Walton (brian@riban.co.uk)
	Date: 2018-12-10
	Provides serial interface, accepting Unix timestamp
	Prints each type of DateTime::format option
*/
#include <SmingCore.h>
#include <pico_partition.h>
#include "pico/bootrom.h"
#include <boot/uf2.h>
#include <Data/CStringArray.h>

namespace
{
CStringArray parseUf2Flags(const Pico::Header& hdr)
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
	Pico::PartitionTableInfo pt;
	Pico::getPartitionTableInfo(pt);
	if(!pt.table_present) {
		printf("there is no partition table\n");
	} else if(pt.partition_count == 0) {
		printf("the partition table is empty\n");
	}

	auto printPermissions = [](const Pico::Header& hdr) {
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
	for(unsigned partIndex = 0; partIndex < pt.partition_count; ++partIndex) {
		Pico::PartitionInfo p;
		Pico::getPartitionInfo(p, partIndex);
		Serial << String(partIndex, DEC, 3) << ":"
			   << "    " << String(p.header.startOffset(), HEX, 8) << " -> " << String(p.header.endOffset(), HEX, 8);
		printPermissions(p.header);
		if(p.header.has_id) {
			Serial << ", id=" << String(p.id, HEX, 16);
		}
		if(p.header.has_name) {
			Serial << ", \"" << p.name() << "\"";
		}

		// print UF2 family ID
		familyIds = parseUf2Flags(p.header);
		for(size_t i = 0; i < p.header.accepts_num_extra_families; i++) {
			familyIds.add(parseUf2Extra(p.extra_family_ids[i]));
		}
		Serial << ", uf2 { " << familyIds.join(", ") << " }";

		Serial << endl;
	}

	return 0;
}

void printBootInfo()
{
	auto lastBootType = rom_get_last_boot_type();

	Serial << "Last boot type " << lastBootType << endl;

	// For a flash update boot, get the flash update base
	boot_info_t boot_info{};
	int rc = rom_get_boot_info(&boot_info);
	if(rc < 0) {
		Serial << "rom_get_boot_info FAILED " << rc << endl;
		return;
	}

	Serial << "diagnostic_partition_index " << boot_info.diagnostic_partition_index << endl;
	Serial << "boot_type " << boot_info.boot_type << endl;
	Serial << "partition " << boot_info.partition << endl;
	Serial << "tbyb_and_update_info " << boot_info.tbyb_and_update_info << endl;
	Serial << "boot_diagnostic " << String(boot_info.boot_diagnostic, HEX) << endl;
	Serial << "reboot_params " << String(boot_info.reboot_params[0], HEX) << ", "
		   << String(boot_info.reboot_params[1], HEX) << endl;
}

} // namespace

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(true);

	printPartitionInfo();
	printBootInfo();
}
