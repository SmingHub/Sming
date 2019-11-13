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

#include <user_config.h>

/// Flash memory access must be aligned and in multiples of 4-byte words
#define INTERNAL_FLASH_WRITE_UNIT_SIZE 4
#define INTERNAL_FLASH_READ_UNIT_SIZE 4

#define FLASH_TOTAL_SEC_COUNT (flashmem_get_size_sectors())

/// Number of flash sectors reserved for system parameters at start
#define SYS_PARAM_SEC_COUNT 4
#define FLASH_WORK_SEC_COUNT (FLASH_TOTAL_SEC_COUNT - SYS_PARAM_SEC_COUNT)

#define INTERNAL_FLASH_SECTOR_SIZE SPI_FLASH_SEC_SIZE
#define INTERNAL_FLASH_SIZE ((FLASH_WORK_SEC_COUNT)*INTERNAL_FLASH_SECTOR_SIZE)
#define INTERNAL_FLASH_START_ADDRESS 0x40200000

/** @brief SPI Flash memory information block.
 * Stored at the beginning of flash memory.
 */
typedef struct
{
	uint8_t unknown0;
	uint8_t unknown1;
    enum
    {
		MODE_QIO = 0,
		MODE_QOUT = 1,
		MODE_DIO = 2,
		MODE_DOUT = 15,
	} mode : 8;
    enum
    {
		SPEED_40MHZ = 0,
		SPEED_26MHZ = 1,
		SPEED_20MHZ = 2,
		SPEED_80MHZ = 15,
	} speed : 4;
    enum
    {
		SIZE_4MBIT = 0,
		SIZE_2MBIT = 1,
		SIZE_8MBIT = 2,
		SIZE_16MBIT = 3,
		SIZE_32MBIT = 4,
	} size : 4;
} STORE_TYPEDEF_ATTR SPIFlashInfo;

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

/** @brief write to flash memory
 *  @param from Buffer to read data from - MUST be word-aligned
 *  @param toaddr Flash address (offset) to write to - MUST be word-aligned
 *  @param size Number of bytes to write - MUST be word-aligned
 *  @retval uint32_t Number of bytes actually written
 *  @note All parameters MUST be aligned to 4-byte word boundaries, **including** the RAM pointer
 */
uint32_t flashmem_write_internal(const void* from, uint32_t toaddr, uint32_t size);

/** @brief Read from flash memory
 *  @param to Buffer to store data - MUST be word-aligned
 *  @param fromaddr Flash address (offset) to read from - MUST be word-aligned
 *  @param size Number of bytes to read - MUST be word-aligned
 *  @retval uint32_t Number of bytes actually read
 *  @note All parameters MUST be aligned to 4-byte word boundaries, **including** the RAM pointer
 */
uint32_t flashmem_read_internal(void* to, uint32_t fromaddr, uint32_t size);

/*
 * @brief Returns the address of the first free block on flash
 * @retval  uint32_t The actual address on flash
 */
uint32_t flashmem_get_first_free_block_address();


#ifdef __cplusplus
}
#endif
