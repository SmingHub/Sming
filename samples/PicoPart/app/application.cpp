#include <SmingCore.h>
#include <pico_partition.h>
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

bool printPartitionInfo()
{
	Pico::PartitionTableInfo pt;
	Pico::getPartitionTableInfo(pt);
	if(!pt.table_present) {
		Serial << "There is no partition table!" << endl;
		return false;
	}

	CStringArray familyIds = parseUf2Flags(pt.unpartitioned_space);
	Serial << "un-partitioned_space: " << pt.unpartitioned_space.getPermissionsString() << " uf2 { "
		   << familyIds.join(", ") << " }" << endl;

	if(pt.partition_count == 0) {
		Serial << "The partition table is empty!" << endl;
		return false;
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

	return true;
}

void printBootInfo()
{
	auto runPart = OtaManager.getRunningPartition();
	auto defaultPart = OtaManager.getBootPartition();
	auto nextPart = OtaManager.getNextBootPartition();
	Serial << "Running " << runPart << endl;
	Serial << "Default " << defaultPart << endl;
	Serial << "Next " << nextPart << endl;

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
		if(!part) {
			Serial << "rom1 not found in partition table!" << endl;
			return;
		}

		Serial << "Rebooting to " << part << endl;
		OtaManager.setBootPartition(part, false);
		System.restart(2000);
	} else if(defaultPart.name() != "rom1") {
		OtaManager.setBootPartition(runPart, true);
		Serial << "Default now " << OtaManager.getBootPartition() << endl;
	}
}

} // namespace

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(true);

	Serial << endl << endl << "Pico Partition sample application" << endl << endl;

	printPartitionInfo();
	printBootInfo();

	auto timer = new SimpleTimer;
	timer->initializeMs<1000>([](void*) { Serial << system_get_time() << " alive" << endl; }).start();
}
