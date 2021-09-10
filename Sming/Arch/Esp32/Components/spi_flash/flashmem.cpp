/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * flashmem.cpp
 *
 ****/

#include <sdkconfig.h>
#include <esp_spi_flash.h>
#include <soc/mmu.h>
#include <esp_flash_partitions.h>
#include <esp_flash.h>
#include <rom/cache.h>

/*
 * Physical <-> Virtual address mapping is handled in `$IDF_COMPONENTS/spi_flash/flash_mmap.c`.
 *
 * See esp32 technical reference.
 */
uint32_t flashmem_get_address(const void* memptr)
{
	auto vaddr = reinterpret_cast<uint32_t>(memptr);
	if(vaddr < SOC_DROM_LOW || vaddr >= SOC_DROM_HIGH) {
		return 0;
	}

	auto offset = vaddr - SOC_MMU_VADDR0_START_ADDR;
	uint32_t page = SOC_MMU_DROM0_PAGES_START + (offset / SPI_FLASH_MMU_PAGE_SIZE);
#if CONFIG_IDF_TARGET_ESP32 && !CONFIG_FREERTOS_UNICORE
	uint32_t entry = DPORT_APP_FLASH_MMU_TABLE[page];
#else
	uint32_t entry = SOC_MMU_DPORT_PRO_FLASH_MMU_TABLE[page];
#endif

	if(entry == SOC_MMU_INVALID_ENTRY_VAL) {
		debug_e("Invalid flash address %p (page %u, entry 0x%08x)", memptr, page, entry);
		return 0;
	}

	entry &= SOC_MMU_ADDR_MASK;
	uint32_t paddr = (entry * SPI_FLASH_MMU_PAGE_SIZE) + (vaddr % SPI_FLASH_MMU_PAGE_SIZE);
	return paddr;
}

uint32_t flashmem_write(const void* from, uint32_t toaddr, uint32_t size)
{
	esp_err_t r = spi_flash_write(toaddr, from, size);
	if(r != ESP_OK) {
		SYSTEM_ERROR("ERROR in flash_write: r=%d at %08X\n", r, toaddr);
		size = 0;
	}

	return size;
}

uint32_t flashmem_read(void* to, uint32_t fromaddr, uint32_t size)
{
	esp_err_t r = spi_flash_read(fromaddr, to, size);
	if(r != ESP_OK) {
		SYSTEM_ERROR("ERROR in flash_read: r=%d at %08X\n", r, fromaddr);
		size = 0;
	}

	return size;
}

bool flashmem_erase_sector(uint32_t sector_id)
{
	debug_e("flashmem_erase_sector(0x%08x)", sector_id);

	return spi_flash_erase_sector(sector_id) == SPI_FLASH_RESULT_OK;
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

uint8_t flashmem_get_size_type()
{
	return flashmem_get_info().size;
}

uint32_t flashmem_get_size_bytes()
{
	return g_rom_flashchip.chip_size;
}

uint16_t flashmem_get_size_sectors()
{
	return flashmem_get_size_bytes() / SPI_FLASH_SEC_SIZE;
}

uint32_t flashmem_find_sector(uint32_t address, uint32_t* pstart, uint32_t* pend)
{
	// All the sectors in the flash have the same size, so just align the address
	uint32_t sect_id = address / INTERNAL_FLASH_SECTOR_SIZE;

	if(pstart)
		*pstart = sect_id * INTERNAL_FLASH_SECTOR_SIZE;
	if(pend)
		*pend = (sect_id + 1) * INTERNAL_FLASH_SECTOR_SIZE - 1;
	return sect_id;
}

uint32_t flashmem_get_sector_of_address(uint32_t addr)
{
	return flashmem_find_sector(addr, NULL, NULL);
}

uint32_t spi_flash_get_id(void)
{
	uint32_t id{0};
	if(esp_flash_read_id(esp_flash_default_chip, &id) != ESP_OK) {
		id = 0;
	}
	return id;
}
