#include <esp_spi_flash.h>

uint16_t flashmem_find_sector(uint32_t address, uint32_t* pstart, uint32_t* pend)
{
	// All the sectors in the flash have the same size, so just align the address
	uint16_t sect_id = address / INTERNAL_FLASH_SECTOR_SIZE;

	if(pstart) {
		*pstart = sect_id * INTERNAL_FLASH_SECTOR_SIZE;
	}
	if(pend) {
		*pend = (sect_id + 1) * INTERNAL_FLASH_SECTOR_SIZE - 1;
	}
	return sect_id;
}

uint16_t flashmem_get_sector_of_address(uint32_t addr)
{
	return addr / INTERNAL_FLASH_SECTOR_SIZE;
}

uint16_t flashmem_get_size_sectors()
{
	return flashmem_get_size_bytes() / INTERNAL_FLASH_SECTOR_SIZE;
}
