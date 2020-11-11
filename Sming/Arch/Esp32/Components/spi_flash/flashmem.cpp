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
#include <soc/soc.h>
#include <soc/dport_reg.h>

/*
 * Physical <-> Virtual address mapping is handled in `$IDF_COMPONENTS/spi_flash/flash_mmap.c`.
 *
 * See esp32 technical reference.
 */
uint32_t flashmem_get_address(const void* memptr)
{
#define VADDR0_START_ADDR SOC_DROM_LOW
#define VADDR1_START_ADDR 0x40000000

	auto vaddr = reinterpret_cast<uint32_t>(memptr);

	uint32_t page;
	if(vaddr >= SOC_DROM_LOW && vaddr < SOC_DROM_HIGH) {
		auto offset = vaddr - VADDR0_START_ADDR;
		page = offset / SPI_FLASH_MMU_PAGE_SIZE;
	} else if(vaddr >= SOC_IROM_LOW && vaddr < SOC_IROM_HIGH) {
		auto offset = vaddr - VADDR1_START_ADDR;
		page = 64 + (offset / SPI_FLASH_MMU_PAGE_SIZE);
	} else {
		return 0;
	}
	uint32_t entry = DPORT_SEQUENCE_REG_READ(uint32_t(&DPORT_APP_FLASH_MMU_TABLE[page]));
	uint32_t paddr = (entry * SPI_FLASH_MMU_PAGE_SIZE) + (vaddr & (SPI_FLASH_MMU_PAGE_SIZE - 1));
	return (entry & 0x0100) ? 0 : paddr;
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
	SPIFlashInfo spi_flash_info STORE_ATTR;
	if(flashmem_read(&spi_flash_info, 0x00000000, sizeof(spi_flash_info)) == 0) {
		memset(&spi_flash_info, 0, sizeof(spi_flash_info));
	}
	return spi_flash_info;
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
