#ifndef __RBOOT_API_H__
#define __RBOOT_API_H__

/** @defgroup rboot rBoot API
 *  @brief      rBoot for ESP8266 API allows runtime code to access the rBoot configuration.
 *              Configuration may be read to use within the main firmware or updated to
 *              affect next boot behavior.
 *  @copyright  2015 Richard A Burton
 *  @author     richardaburton@gmail.com
 *  @author     OTA code based on SDK sample from Espressif
 *  @license    See licence.txt for license terms.
 *  @{
*/

#include <rboot.h>

#ifdef __cplusplus
extern "C" {
#endif

/**	@brief  Structure defining flash write status
 *  @note   The user application should not modify the contents of this
 *          structure.
 *	@see    rboot_write_flash
*/
typedef struct {
	uint32 start_addr;
	uint32 start_sector;
	//uint32 max_sector_count;
	uint32 last_sector_erased;
	uint8 extra_count;
	uint8 extra_bytes[4];
} rboot_write_status;

/**	@brief	Read rBoot configuration from flash
 *	@retval rboot_config Copy of the rBoot configuration
 *  @note   Returns rboot_config (defined in rboot.h) allowing you to modify any values
 *          in it, including the ROM layout.
*/
rboot_config ICACHE_FLASH_ATTR rboot_get_config(void);

/**	@brief	Write rBoot configuration to flash memory
 *	@param	conf pointer to a rboot_config structure containing configuration to save
 *	@retval bool True on success
 *  @note   Saves the rboot_config structure back to configuration sector (BOOT_CONFIG_SECTOR)
 *          of the flash, while maintaining the contents of the rest of the sector.
 *          You can use the rest of this sector for your app settings, as long as you
 *          protect this structure when you do so.
*/
bool ICACHE_FLASH_ATTR rboot_set_config(rboot_config *conf);

/**	@brief	Get index of current ROM
 *	@retval uint8 Index of the current ROM
 *  @note   Get the currently selected boot ROM (the currently running ROM, as long as
 *          you haven't changed it since boot)
*/
uint8 ICACHE_FLASH_ATTR rboot_get_current_rom(void);

/**	@brief	Set the index of current ROM
 *	@param	rom The index of the ROM to use on next boot
 *	@retval	bool True on success
 *  @note   Set the current boot ROM, which will be used when next restarted.
 *  @note   This function re-writes the whole configuration to flash memory (not just the current ROM index)
*/
bool ICACHE_FLASH_ATTR rboot_set_current_rom(uint8 rom);

/**	@brief  Initialise flash write process
 *	@param  start_addr Address on the SPI flash to begin write to
 *  @note   Call once before starting to pass data to write to flash memory with rboot_write_flash function.
 *          start_addr is the address on the SPI flash to write from. Returns a status structure which
 *          must be passed back on each write. The contents of the structure should not
 *          be modified by the calling code.
*/
rboot_write_status ICACHE_FLASH_ATTR rboot_write_init(uint32 start_addr);

/**	@brief  Write data to flash memory
 *	@param  status Pointer to rboot_write_status structure defining the write status
 *  @param  data Pointer to a block of uint8 data elements to be written to flash
 *  @param  len Quantity of uint8 data elements to write to flash
 *  @note   Call repeatedly to write data to the flash, starting at the address
 *  specified on the prior call to rboot_write_init. Current write position is
 *  tracked automatically. This method is likely to be called each time a packet
 *  of OTA data is received over the network.
 *  @note   Call rboot_write_init before calling this function to get the rboot_write_status structure
*/
bool ICACHE_FLASH_ATTR rboot_write_flash(rboot_write_status *status, uint8 *data, uint16 len);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
