/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SpiFlash.cpp
 *
 ****/

#include "include/Storage/SpiFlash.h"
#include "include/Storage/partition_info.h"
#include <esp_spi_flash.h>

namespace Storage
{
DEFINE_FSTR(FS_SPIFLASH, "spiFlash")
SpiFlash* spiFlash;

String SpiFlash::getName() const
{
	return FS_SPIFLASH;
}

uint32_t SpiFlash::getId() const
{
	return spi_flash_get_id();
}

size_t SpiFlash::getBlockSize() const
{
	return SPI_FLASH_SEC_SIZE;
}

size_t SpiFlash::getSize() const
{
	return flashmem_get_size_bytes();
}

bool SpiFlash::read(uint32_t address, void* dst, size_t size)
{
	size_t readCount = flashmem_read(dst, address, size);
	return readCount == size;
}

bool SpiFlash::write(uint32_t address, const void* src, size_t size)
{
	size_t writeCount = flashmem_write(src, address, size);
	return writeCount == size;
}

bool SpiFlash::erase_range(uint32_t address, size_t size)
{
	if(address % SPI_FLASH_SEC_SIZE != 0 || size % SPI_FLASH_SEC_SIZE != 0) {
		debug_e("[Partition] erase address/size misaligned: 0x%08x / 0x%08x", address, size);
		return false;
	}

	auto sec = address / SPI_FLASH_SEC_SIZE;
	auto end = (address + size) / SPI_FLASH_SEC_SIZE;
	while(sec < end) {
		if(!flashmem_erase_sector(sec)) {
			return false;
		}
		++sec;
	}

	return true;
}

} // namespace Storage
