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

#include_next <esp_spi_flash.h>
#include <esp32/rom/spi_flash.h>
#include <esp_app_format.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <user_config.h>

/**
 * @defgroup flash Flash Memory Support
 * @defgroup spi_flash SPI Flash API
 * @ingroup flash
 * @{
 */

#define SPI_FLASH_RESULT_OK 0

/// Flash memory access must be aligned and in multiples of 4-byte words
#define INTERNAL_FLASH_WRITE_UNIT_SIZE 4
#define INTERNAL_FLASH_READ_UNIT_SIZE 4

#define FLASH_TOTAL_SEC_COUNT (flashmem_get_size_sectors())

/// Number of flash sectors reserved for system parameters at start
#define SYS_PARAM_SEC_COUNT 4
#define FLASH_WORK_SEC_COUNT (FLASH_TOTAL_SEC_COUNT - SYS_PARAM_SEC_COUNT)

#define INTERNAL_FLASH_SECTOR_SIZE SPI_FLASH_SEC_SIZE
#define INTERNAL_FLASH_SIZE ((FLASH_WORK_SEC_COUNT)*INTERNAL_FLASH_SECTOR_SIZE)

typedef enum {
	MODE_QIO = ESP_IMAGE_SPI_MODE_QIO,
	MODE_QOUT = ESP_IMAGE_SPI_MODE_QOUT,
	MODE_DIO = ESP_IMAGE_SPI_MODE_DIO,
	MODE_DOUT = ESP_IMAGE_SPI_MODE_DOUT,
	MODE_FAST_READ = ESP_IMAGE_SPI_MODE_FAST_READ,
	MODE_SLOW_READ = ESP_IMAGE_SPI_MODE_SLOW_READ,
} SPIFlashMode;

typedef enum {
	SPEED_40MHZ = ESP_IMAGE_SPI_SPEED_40M,
	SPEED_26MHZ = ESP_IMAGE_SPI_SPEED_26M,
	SPEED_20MHZ = ESP_IMAGE_SPI_SPEED_20M,
	SPEED_80MHZ = ESP_IMAGE_SPI_SPEED_80M,
} SPIFlashSpeed;

typedef enum {
	SIZE_1MBIT = ESP_IMAGE_FLASH_SIZE_1MB,
	SIZE_2MBIT = ESP_IMAGE_FLASH_SIZE_2MB,
	SIZE_4MBIT = ESP_IMAGE_FLASH_SIZE_4MB,
	SIZE_8MBIT = ESP_IMAGE_FLASH_SIZE_8MB,
	SIZE_16MBIT = ESP_IMAGE_FLASH_SIZE_16MB,
	SIZE_32MBIT = 0xFF, ///< Not listed
} SPIFlashSize;

/**
 * @brief SPI Flash memory information block.
 * Copied from bootloader header.
 * See `esp_image_header_t`.
 */
typedef struct {
	SPIFlashMode mode;
	SPIFlashSpeed speed;
	SPIFlashSize size;
} SPIFlashInfo;

/** @brief Obtain the flash memory address for a memory pointer
 *  @param memptr
 *  @retval uint32_t Offset from start of flash memory
 *  @note If memptr is not in valid flash memory it will return an offset which exceeds
 *  the internal flash memory size.
 *  @note The flash location is dependent on where rBoot has mapped the firmware.
 */
uint32_t flashmem_get_address(const void* memptr);

/** @brief Write a block of data to flash
 *  @param from Buffer to obtain data from
 *  @param toaddr Flash location to start writing
 *  @param size Number of bytes to write
 *  @retval uint32_t Number of bytes written
 *  @note None of the parameters need to be aligned
 */
uint32_t flashmem_write(const void* from, uint32_t toaddr, uint32_t size);

/** @brief Read a block of data from flash
 *  @param to Buffer to store data
 *  @param fromaddr Flash location to start reading
 *  @param size Number of bytes to read
 *  @retval uint32_t Number of bytes written
 *  @note none of the parameters need to be aligned
 */
uint32_t flashmem_read(void* to, uint32_t fromaddr, uint32_t size);

/** @brief Erase a single flash sector
 *  @param sector_id the sector to erase
 *  @retval true on success
 */
bool flashmem_erase_sector(uint32_t sector_id);

/** @brief Get flash memory information block
 *  @retval SPIFlashInfo Information block
 */
SPIFlashInfo flashmem_get_info();

/** @brief Returns a number indicating the size of flash memory chip
 *  @retval uint8_t See SpiFlashInfo.size field for possible values
 */
uint8_t flashmem_get_size_type();

/** @brief get the total flash memory size
 *  @retval uint32_t Size in bytes
 */
uint32_t flashmem_get_size_bytes();

/** @brief Get the total number of flash sectors
 *  @retval uint16_t Sector count
 */
uint16_t flashmem_get_size_sectors();

/** @brief Helper function: find the flash sector in which an address resides
 *  @param address
 *  @param pstart OUT/OPTIONAL: Start of sector containing the given address
 *  @param pend OUT/OPTIONAL: Last address in sector
 *  @retval uint32_t Sector number for the given address
 *  @note Optional parameters may be null
 */
uint32_t flashmem_find_sector(uint32_t address, uint32_t* pstart, uint32_t* pend);

/** @brief Get sector number containing the given address
 *  @param addr
 *  @retval uint32_t sector number
 */
uint32_t flashmem_get_sector_of_address(uint32_t addr);

/** @} */

#ifdef __cplusplus
}
#endif
