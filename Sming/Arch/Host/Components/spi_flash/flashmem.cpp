/**
 * flashmem.c
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "flashmem.h"
#include <string.h>
#include <esp_spi_flash.h>

static int flashFile = -1;
static size_t flashFileSize = 0x400000U;
static char flashFileName[256];
static const char defaultFlashFileName[] = "flash.bin";

// Top bit of flash address is set to indicate it's actually program memory
static constexpr uint32_t FLASHMEM_REAL_BIT = 0x80000000U;
static constexpr uint32_t FLASHMEM_REAL_MASK = ~FLASHMEM_REAL_BIT;

#define SPI_FLASH_SEC_SIZE 4096

#define CHECK_ALIGNMENT(_x) assert(((uint32_t)(_x)&0x00000003) == 0)

#define CHECK_RANGE(_addr, _size)                                                                                      \
	if((_addr) + (_size) > flashFileSize) {                                                                            \
		hostmsg("addr = 0x%08x, size = 0x%08x", _addr, _size);                                                         \
		return false;                                                                                                  \
	}

bool host_flashmem_init(FlashmemConfig& config)
{
	if(config.filename != nullptr) {
		strncpy(flashFileName, config.filename, sizeof(flashFileName));
		flashFileName[sizeof(flashFileName) - 1] = '\0';
	} else {
		size_t len = getHostAppDir(flashFileName, sizeof(flashFileName));
		if(len + sizeof(defaultFlashFileName) > sizeof(flashFileName)) {
			return false;
		}
		memcpy(&flashFileName[len], defaultFlashFileName, sizeof(defaultFlashFileName));
		config.filename = flashFileName;
	}

	flashFile = open(flashFileName, O_CREAT | O_RDWR | O_BINARY, 0644);
	if(flashFile < 0) {
		hostmsg("Error opening \"%s\"", flashFileName);
		return false;
	}

	int res = lseek(flashFile, 0, SEEK_END);
	if(res < 0) {
		hostmsg("Error seeking \"%s\"", flashFileName);
		close(flashFile);
		flashFile = -1;
		return false;
	}

	if(res == 0) {
		size_t size = config.createSize ?: flashFileSize;
		res = lseek(flashFile, size, SEEK_SET);
		if(res != int(size)) {
			hostmsg("Error seeking beyond end of file \"%s\"", flashFileName);
		} else if(ftruncate(flashFile, size) < 0) {
			hostmsg("Error truncating \"%s\" to %u bytes", flashFileName, size);
		} else {
			hostmsg("Created blank \"%s\", %u bytes", flashFileName, size);
		}
	} else {
		hostmsg("Opened \"%s\", size = 0x%08x", flashFileName, res);
	}

	flashFileSize = res;
	config.createSize = flashFileSize;

	return true;
}

void host_flashmem_cleanup()
{
	close(flashFile);
	flashFile = -1;
	hostmsg("Closed \"%s\"", flashFileName);
}

static int readFlashFile(uint32_t offset, void* buffer, size_t count)
{
	if(flashFile < 0) {
		return -1;
	}
	int res = lseek(flashFile, offset, SEEK_SET);
	return (res < 0) ? res : read(flashFile, buffer, count);
}

static int writeFlashFile(uint32_t offset, const void* data, size_t count)
{
	if(flashFile < 0) {
		return -1;
	}
	int res = lseek(flashFile, offset, SEEK_SET);
	return (res < 0) ? res : write(flashFile, data, count);
}

//SPIFlashInfo flashmem_get_info()
//{
//}

//uint8_t flashmem_get_size_type()
//{
//}

uint32 spi_flash_get_id(void)
{
	return 0xFA1E0008;
}

uint32_t flashmem_get_size_bytes()
{
	return flashFileSize;
}

uint16_t flashmem_get_size_sectors()
{
	return flashFileSize / SPI_FLASH_SEC_SIZE;
}

uint32_t flashmem_write_internal(const void* from, uint32_t toaddr, uint32_t size)
{
	CHECK_ALIGNMENT(from);
	CHECK_ALIGNMENT(toaddr);
	CHECK_ALIGNMENT(size);
	CHECK_RANGE(toaddr, size);
	return flashmem_write(from, toaddr, size);
}

static bool readRealMem(void* buffer, uint32_t addr, uint32_t count)
{
	if((addr & FLASHMEM_REAL_BIT) == 0) {
		return false;
	}

	auto memPtr = reinterpret_cast<const void*>(addr & FLASHMEM_REAL_MASK);
	memcpy(buffer, memPtr, count);
	return true;
}

uint32_t flashmem_read_internal(void* to, uint32_t fromaddr, uint32_t size)
{
	if(readRealMem(to, fromaddr, size)) {
		return size;
	}

	CHECK_ALIGNMENT(to);
	CHECK_ALIGNMENT(fromaddr);
	CHECK_ALIGNMENT(size);
	CHECK_RANGE(fromaddr, size);
	int res = readFlashFile(fromaddr, to, size);
	return (res < 0) ? 0 : res;
}

uint32_t flashmem_write(const void* from, uint32_t toaddr, uint32_t size)
{
	CHECK_RANGE(toaddr, size);
	int res = writeFlashFile(toaddr, from, size);
	return (res < 0) ? 0 : res;
}

uint32_t flashmem_read(void* to, uint32_t fromaddr, uint32_t size)
{
	if(readRealMem(to, fromaddr, size)) {
		return size;
	}

	CHECK_RANGE(fromaddr, size);
	int res = readFlashFile(fromaddr, to, size);
	return (res < 0) ? 0 : res;
}

uint32_t flashmem_get_sector_of_address(uint32_t addr)
{
	return addr / INTERNAL_FLASH_SECTOR_SIZE;
}

bool flashmem_erase_sector(uint32_t sector_id)
{
	uint32_t addr = sector_id * INTERNAL_FLASH_SECTOR_SIZE;
	CHECK_RANGE(addr, INTERNAL_FLASH_SECTOR_SIZE);
	uint8_t tmp[INTERNAL_FLASH_SECTOR_SIZE];
	memset(tmp, 0xFF, sizeof(tmp));
	return writeFlashFile(addr, tmp, sizeof(tmp)) == sizeof(tmp);
}

uint32_t flashmem_get_address(const void* memptr)
{
	return reinterpret_cast<uint32_t>(memptr) | FLASHMEM_REAL_BIT;
}
