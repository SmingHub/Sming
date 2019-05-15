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

/*
 * rBoot uses different spiffs organization and we need to override this method
 * during application compile time  in order to make automatic
 * mounting with `spiffs_mount()` work as expected.
 */
spiffs_config spiffs_get_storage_config()
{
  spiffs_config cfg = {0};
  u32_t max_allowed_sector, requested_sector;

#ifdef RBOOT_SPIFFS_0
  cfg.phys_addr = RBOOT_SPIFFS_0;
#elif RBOOT_SPIFFS_1
  cfg.phys_addr = RBOOT_SPIFFS_1;
#else
#error "Define either RBOOT_SPIFFS_0 or RBOOT_SPIFFS_1"
#endif

  cfg.phys_addr &= 0xFFFFF000; // get the start address of the sector

  max_allowed_sector = flashmem_get_sector_of_address(INTERNAL_FLASH_SIZE - 1);
  requested_sector  = flashmem_get_sector_of_address((cfg.phys_addr + SPIFF_SIZE) - 1);
  if(requested_sector > max_allowed_sector) {
      debug_w("The requested SPIFFS size is too big.");
      requested_sector = max_allowed_sector;
  }
  cfg.phys_size = ((requested_sector + 1) * INTERNAL_FLASH_SECTOR_SIZE) -  ( ( u32_t )cfg.phys_addr); // get the max size until the sector end.

  cfg.phys_erase_block = INTERNAL_FLASH_SECTOR_SIZE; // according to datasheet
  cfg.log_block_size = INTERNAL_FLASH_SECTOR_SIZE * 2; // Important to make large
  cfg.log_page_size = LOG_PAGE_SIZE; // as we said
  return cfg;
}
