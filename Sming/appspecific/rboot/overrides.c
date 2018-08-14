#include "spiffs_sming.h"

/*
 * rBoot uses different spiffs organization and we need to override this method
 * during application compile time  in order to make automatic
 * mounting with `spiffs_mount()` work as expected.
 */
spiffs_config spiffs_get_storage_config()
{
  spiffs_config cfg = {0};
#ifdef RBOOT_SPIFFS_0
  cfg.phys_addr = RBOOT_SPIFFS_0;
#elif RBOOT_SPIFFS_1
  cfg.phys_addr = RBOOT_SPIFFS_1;
#else
#error "Define either RBOOT_SPIFFS_0 or RBOOT_SPIFFS_1"
#endif
  cfg.phys_size = SPIFF_SIZE;
  cfg.phys_erase_block = INTERNAL_FLASH_SECTOR_SIZE; // according to datasheet
  cfg.log_block_size = INTERNAL_FLASH_SECTOR_SIZE * 2; // Important to make large
  cfg.log_page_size = SPIFFS_LOG_PAGE_SIZE; // as we said
  return cfg;
}
