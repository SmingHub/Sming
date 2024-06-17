/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 ****/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <spi_flash.h>
#include <esp_attr.h>

#define INTERNAL_FLASH_SECTOR_SIZE 4096
#define INTERNAL_FLASH_START_ADDRESS 0x40200000

typedef enum {
	MODE_QIO = 0,
	MODE_QOUT = 1,
	MODE_DIO = 2,
	MODE_DOUT = 15,
	MODE_SLOW_READ = 0xFE, ///< Not supported
	MODE_FAST_READ = 0xFF, ///< Not supported
} SPIFlashMode;

typedef enum {
	SPEED_40MHZ = 0,
	SPEED_26MHZ = 1,
	SPEED_20MHZ = 2,
	SPEED_80MHZ = 15,
} SPIFlashSpeed;

typedef enum {
	SIZE_4MBIT = 0,
	SIZE_2MBIT = 1,
	SIZE_8MBIT = 2,
	SIZE_16MBIT = 3,
	SIZE_32MBIT = 4,
	SIZE_1MBIT = 0xFF, ///< Not supported
} SPIFlashSize;

/** @brief write to flash memory
 *  @param from Buffer to read data from - MUST be word-aligned
 *  @param toaddr Flash address (offset) to write to - MUST be word-aligned
 *  @param size Number of bytes to write - MUST be word-aligned
 *  @retval uint32_t Number of bytes actually written
 *  @note All parameters MUST be aligned to 4-byte word boundaries, **including** the RAM pointer
 */
uint32_t flashmem_write_internal(const void* from, uintptr_t toaddr, uint32_t size);

/** @brief Read from flash memory
 *  @param to Buffer to store data - MUST be word-aligned
 *  @param fromaddr Flash address (offset) to read from - MUST be word-aligned
 *  @param size Number of bytes to read - MUST be word-aligned
 *  @retval uint32_t Number of bytes actually read
 *  @note All parameters MUST be aligned to 4-byte word boundaries, **including** the RAM pointer
 */
uint32_t flashmem_read_internal(void* to, uintptr_t fromaddr, uint32_t size);

#ifdef __cplusplus
}
#endif

#include <flashmem.h>
