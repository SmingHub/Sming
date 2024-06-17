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

#include <esp_idf_version.h>
#if ESP_IDF_VERSION_MAJOR < 5
#include_next <esp_spi_flash.h>
#else
#include <spi_flash_mmap.h>
#endif
#include <rom/spi_flash.h>
#include <esp_app_format.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INTERNAL_FLASH_SECTOR_SIZE SPI_FLASH_SEC_SIZE

typedef enum {
	MODE_QIO = ESP_IMAGE_SPI_MODE_QIO,
	MODE_QOUT = ESP_IMAGE_SPI_MODE_QOUT,
	MODE_DIO = ESP_IMAGE_SPI_MODE_DIO,
	MODE_DOUT = ESP_IMAGE_SPI_MODE_DOUT,
	MODE_FAST_READ = ESP_IMAGE_SPI_MODE_FAST_READ,
	MODE_SLOW_READ = ESP_IMAGE_SPI_MODE_SLOW_READ,
} SPIFlashMode;

typedef enum {
	SPEED_40MHZ = 0,	// ESP_IMAGE_SPI_SPEED_40M
	SPEED_26MHZ = 1,	// ESP_IMAGE_SPI_SPEED_26M
	SPEED_20MHZ = 2,	// ESP_IMAGE_SPI_SPEED_20M
	SPEED_80MHZ = 0x0f, // ESP_IMAGE_SPI_SPEED_80M
} SPIFlashSpeed;

typedef enum {
	SIZE_1MBIT = ESP_IMAGE_FLASH_SIZE_1MB,
	SIZE_2MBIT = ESP_IMAGE_FLASH_SIZE_2MB,
	SIZE_4MBIT = ESP_IMAGE_FLASH_SIZE_4MB,
	SIZE_8MBIT = ESP_IMAGE_FLASH_SIZE_8MB,
	SIZE_16MBIT = ESP_IMAGE_FLASH_SIZE_16MB,
	SIZE_32MBIT = 0xFF, ///< Not listed
} SPIFlashSize;

#ifdef __cplusplus
}
#endif

#include <flashmem.h>
