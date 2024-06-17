/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Based on NodeMCU platform_flash
 * https://github.com/nodemcu/nodemcu-firmware
 *
 ****/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_system.h>
#include <sys/pgmspace.h>

#define INTERNAL_FLASH_SECTOR_SIZE 4096
#define INTERNAL_FLASH_START_ADDRESS FLASH_BASE

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

/*
 * @brief Read flash SFDP device information (standard for Serial Flash Discoverable Parameters)
 * @param addr First memory location to read
 * @param buffer Buffer for data
 * @param count Number of bytes to read
 * @ingroup flash
 */
void flashmem_sfdp_read(uint32_t addr, void* buffer, size_t count);

#ifdef __cplusplus
}
#endif

#include <flashmem.h>
