/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * host.cpp - rboot support code for emulator
 *
 ****/

#include <esp_spi_flash.h>
#include <rboot-api.h>
#include <hostlib/hostmsg.h>
#include <WString.h>

static uint32_t SPIRead(uint32_t addr, void* outptr, uint32_t len)
{
	return (flashmem_read(outptr, addr, len) == len) ? 0 : 1;
}

#define SPIEraseSector(sector) flashmem_erase_sector(sector)
#define echof(fmt, ...) host_printf(fmt, ##__VA_ARGS__)

#include <partition.h>

/*
 * Called from emulator startup code after flash has been initialised.
 */
void host_init_bootloader()
{
	rboot_config romconf = rboot_get_config();

	// fresh install or old version?
	bool init = false;
	if(romconf.magic != BOOT_CONFIG_MAGIC) {
		hostmsg("MAGIC mismatch");
		init = true;
	} else if(romconf.version != BOOT_CONFIG_VERSION) {
		hostmsg("VERSION mismatch: %u found, current %u", romconf.version, BOOT_CONFIG_VERSION);
		init = true;
	}

	if(init) {
		// create a default config for a standard 2 rom setup
		memset(&romconf, 0, sizeof(romconf));
		romconf.magic = BOOT_CONFIG_MAGIC;
		romconf.version = BOOT_CONFIG_VERSION;
	}

	// Read ROM locations from partition table
	bool config_changed = false;
	if(scan_partitions(&romconf)) {
		config_changed = true;
	}

	if(romconf.count == 0) {
		hostmsg("ERROR! No App partitions found\r\n");
		return;
	}

	if(init || config_changed) {
		bool ok = rboot_set_config(&romconf);
		hostmsg("Update rBoot config: %s", ok ? "OK" : "FAIL");
	}

	String addr;
	for(unsigned i = 0; i < romconf.count; ++i) {
		if(i > 0) {
			addr += ", ";
		}
		addr += '#' + String(i) + ": 0x" + String(romconf.roms[i], 16);
	}
	hostmsg("ROM count = %u, current = %u, %s", romconf.count, romconf.current_rom, addr.c_str());
}
