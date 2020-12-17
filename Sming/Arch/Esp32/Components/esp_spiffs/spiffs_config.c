/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * rboot-overrides.c
 *
 ****/

#include "spiffs_sming.h"
#include <esp_spi_flash.h>
#include <esp_partition.h>

/*
 * rBoot uses different spiffs organization and we need to override this method
 * during application compile time  in order to make automatic
 * mounting with `spiffs_mount()` work as expected.
 */
spiffs_config spiffs_get_storage_config()
{
	spiffs_config cfg = {0};

	const esp_partition_t* partition =
		esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);

	if(partition == NULL) {
		debug_w("No SPIFFS partition registered");
	} else {
		cfg.phys_addr = partition->address;
		cfg.phys_size = partition->size;
		debug_w("SPIFFS partition found at 0x%08x, size 0x%08x", cfg.phys_addr, cfg.phys_size);
		// TODO: Check partition->flash_chip is valid?
	}

	return cfg;
}
