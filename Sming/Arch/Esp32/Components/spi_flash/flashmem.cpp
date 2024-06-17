/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * flashmem.cpp
 *
 ****/

#include <esp_spi_flash.h>
#include <soc/mmu.h>
#include <esp_flash_partitions.h>
#include <esp_flash.h>
#include <esp_task_wdt.h>
#include <rom/cache.h>
#include <esp_systemapi.h>

uint32_t flashmem_write(const void* from, flash_addr_t toaddr, uint32_t size)
{
	esp_err_t r = esp_flash_write(esp_flash_default_chip, from, toaddr, size);
	if(r != ESP_OK) {
		SYSTEM_ERROR("ERROR in flash_write: r=%d at %08X\n", r, toaddr);
		size = 0;
	}

	return size;
}

uint32_t flashmem_read(void* to, flash_addr_t fromaddr, uint32_t size)
{
	esp_err_t r = esp_flash_read(esp_flash_default_chip, to, fromaddr, size);
	if(r != ESP_OK) {
		SYSTEM_ERROR("ERROR in flash_read: r=%d at %08X\n", r, fromaddr);
		size = 0;
	}

	return size;
}

bool flashmem_erase_sector(flash_sector_t sector_id)
{
	esp_task_wdt_reset();

	debug_d("flashmem_erase_sector(0x%08x)", sector_id);

	return esp_flash_erase_region(esp_flash_default_chip, sector_id * SPI_FLASH_SEC_SIZE, SPI_FLASH_SEC_SIZE) == ESP_OK;
}

SPIFlashInfo flashmem_get_info()
{
	SPIFlashInfo info{};
	esp_image_header_t hdr{};
	flashmem_read(&hdr, ESP_BOOTLOADER_OFFSET, sizeof(hdr));
	if(hdr.magic == ESP_IMAGE_HEADER_MAGIC) {
		info.mode = SPIFlashMode(hdr.spi_mode);
		info.speed = SPIFlashSpeed(hdr.spi_speed);
		info.size = SPIFlashSize(hdr.spi_size);
	}

	return info;
}

SPIFlashSize flashmem_get_size_type()
{
	return flashmem_get_info().size;
}

flash_addr_t flashmem_get_size_bytes()
{
	return g_rom_flashchip.chip_size;
}

flash_addr_t flashmem_get_address(const void* memptr)
{
	auto phys = spi_flash_cache2phys(memptr);
	return (phys == SPI_FLASH_CACHE2PHYS_FAIL) ? 0 : phys;
}

uint32_t spi_flash_get_id(void)
{
	uint32_t id{0};
	if(esp_flash_read_id(esp_flash_default_chip, &id) != ESP_OK) {
		id = 0;
	}
	return id;
}
