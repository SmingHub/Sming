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

/*
 * Called from emulator startup code after flash has been initialised.
 */
void host_init_bootloader()
{
	rboot_config romconf = rboot_get_config();

	bool init;

	// fresh install or old version?
	if(romconf.magic != BOOT_CONFIG_MAGIC) {
		hostmsg("MAGIC mismatch");
		init = true;
	} else if(romconf.version != BOOT_CONFIG_VERSION) {
		hostmsg("VERSION mismatch: %u found, current %u", romconf.version, BOOT_CONFIG_VERSION);
		init = true;
	} else {
		// OK
		init = false;
	}

	if(init) {
		// create a default config for a standard 2 rom setup
		memset(&romconf, 0, sizeof(romconf));
		romconf.magic = BOOT_CONFIG_MAGIC;
		romconf.version = BOOT_CONFIG_VERSION;
		romconf.count = 2;
		romconf.roms[0] = SECTOR_SIZE * (BOOT_CONFIG_SECTOR + 1);
#ifdef BOOT_ROM1_ADDR
		romconf.roms[1] = BOOT_ROM1_ADDR;
#else
		size_t flashsize = flashmem_get_size_bytes();
		romconf.roms[1] = (flashsize / 2) + (SECTOR_SIZE * (BOOT_CONFIG_SECTOR + 1));
#endif
		bool ok = rboot_set_config(&romconf);
		hostmsg("Write default config: %s", ok ? "OK" : "FAIL");
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
