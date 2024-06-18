#include <esp_spi_flash.h>

flash_sector_t flashmem_find_sector(flash_addr_t address, flash_addr_t* pstart, flash_addr_t* pend)
{
	// All the sectors in the flash have the same size, so just align the address
	flash_sector_t sect_id = address / INTERNAL_FLASH_SECTOR_SIZE;

	if(pstart) {
		*pstart = sect_id * INTERNAL_FLASH_SECTOR_SIZE;
	}
	if(pend) {
		*pend = (sect_id + 1) * INTERNAL_FLASH_SECTOR_SIZE - 1;
	}
	return sect_id;
}

flash_sector_t flashmem_get_sector_of_address(flash_addr_t addr)
{
	return addr / INTERNAL_FLASH_SECTOR_SIZE;
}

flash_sector_t flashmem_get_size_sectors()
{
	return flashmem_get_size_bytes() / INTERNAL_FLASH_SECTOR_SIZE;
}
