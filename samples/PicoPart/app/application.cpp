/**	Basic_DateTime sample application
	Author: Brian Walton (brian@riban.co.uk)
	Date: 2018-12-10
	Provides serial interface, accepting Unix timestamp
	Prints each type of DateTime::format option
*/
#include <SmingCore.h>
#include <pico_partition.h>
#include <pico/multicore.h>
#include <pico/bootrom.h>
#include <boot/uf2.h>
#include <Data/CStringArray.h>
#include <Ota/Manager.h>

namespace
{
CStringArray parseUf2Flags(const Pico::PartitionHeader& hdr)
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

	CStringArray familyIds = parseUf2Flags(pt.unpartitioned_space);
	Serial << "un-partitioned_space: " << pt.unpartitioned_space.getPermissionsString() << " uf2 { "
		   << familyIds.join(", ") << " }" << endl;

	if(pt.partition_count == 0) {
		return 0;
	}
	Serial << "partitions:" << endl;
	for(unsigned partIndex = 0; partIndex < pt.partition_count; ++partIndex) {
		Pico::PartitionInfo p;
		Pico::getPartitionInfo(p, partIndex);
		Serial << String(partIndex, DEC, 3) << ": 0x" << String(p.header.startOffset(), HEX, 8) << " -> 0x"
			   << String(p.header.endOffset(), HEX, 8) << " " << p.header.getPermissionsString();
		if(p.header.has_id) {
			Serial << ", id = 0x" << String(p.id, HEX, 16);
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
	auto runPart = OtaManager.getRunningPartition();
	auto defaultPart = OtaManager.getBootPartition();
	Serial << "Running " << runPart << endl;
	Serial << "Default " << defaultPart << endl;

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

	// if(boot_info.boot_type != BOOT_TYPE_FLASH_UPDATE) {
	if(runPart.name() == "rom0") {
		auto part = Storage::findPartition("rom1");
		Serial << "Rebooting to " << part << endl;
		OtaManager.setBootPartition(part, false);
		System.restart(2000);
		return;
		// int rc = rom_reboot(REBOOT2_FLAG_REBOOT_TYPE_FLASH_UPDATE | REBOOT2_FLAG_NO_RETURN_ON_SUCCESS, 1000,
		// 					XIP_BASE + 0x100000, 0);
		// Serial << "rom_reboot: " << rc << endl;
	}

	if(boot_info.tbyb_and_update_info & BOOT_TBYB_AND_UPDATE_FLAG_BUY_PENDING) {
		OtaManager.setBootPartition(runPart, true);
		Serial << "Default now " << OtaManager.getBootPartition() << endl;

#if 0
		/*
			explicit buy does two things:

				1. Clear the TBYB flag in the current image
				2. Wipe first sector of other application partition


		 */
		uint8_t buffer[4096];
#if 0
		// Without this workaround, rom_explicit_buy fails
		multicore_launch_core1([]() {
			flash_safe_execute_core_init();
			for(;;) {
			}
		});
		int rc = rom_explicit_buy(buffer, sizeof(buffer));
#else
		// Bypass flash safety. Flash access via core1 is a bad idea
		int rc{0};
		rom_helper_explicit_buy_params_t params = {
			.buffer = buffer,
			.buffer_size = sizeof(buffer),
			.res = &rc,
		};
		rom_helper_explicit_buy(&params);

#endif
		Serial << "rom_explicit_buy: " << rc << endl;
#endif
	}
}

} // namespace

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(true);

	printPartitionInfo();
	printBootInfo();

	auto timer = new SimpleTimer;
	timer->initializeMs<1000>([](void*) { Serial << system_get_time() << " alive" << endl; }).start();
}
