/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spiffs_sming.cpp
 *
 ****/

#include "spiffs_sming.h"
#include <esp_spi_flash.h>
extern "C" {
#include "spiffs/src/spiffs_nucleus.h"
}

spiffs _filesystemStorageHandle;

#ifndef SPIFF_FILEDESC_COUNT
#define SPIFF_FILEDESC_COUNT 7
#endif

namespace
{
uint8_t spiffs_work_buf[LOG_PAGE_SIZE * 2];
uint8_t spiffs_fds[sizeof(spiffs_fd) * SPIFF_FILEDESC_COUNT];
uint8_t spiffs_cache_buf[(LOG_PAGE_SIZE + 32) * 4];

s32_t api_spiffs_read(u32_t addr, u32_t size, u8_t* dst)
{
	return (flashmem_read(dst, addr, size) == size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

s32_t api_spiffs_write(u32_t addr, u32_t size, u8_t* src)
{
	//debugf("api_spiffs_write");
	return (flashmem_write(src, addr, size) == size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

s32_t api_spiffs_erase(u32_t addr, u32_t size)
{
	debugf("api_spiffs_erase");
	uint32_t sect_first = flashmem_get_sector_of_address(addr);
	uint32_t sect_last = sect_first;
	while(sect_first <= sect_last) {
		if(!flashmem_erase_sector(sect_first++)) {
			return SPIFFS_ERR_INTERNAL;
		}
	}
	return SPIFFS_OK;
}

bool tryMount(const spiffs_config& cfg)
{
	int res = SPIFFS_mount(&_filesystemStorageHandle, const_cast<spiffs_config*>(&cfg), spiffs_work_buf, spiffs_fds,
						   sizeof(spiffs_fds), spiffs_cache_buf, sizeof(spiffs_cache_buf), nullptr);
	debugf("mount res: %d", res);

	return res >= 0;
}

bool spiffs_format_internal(const spiffs_config& cfg)
{
	spiffs_unmount();
	if(tryMount(cfg)) {
		spiffs_unmount();
	}

	int res = SPIFFS_format(&_filesystemStorageHandle);
	return res >= 0;
}

bool spiffs_mount_internal(const spiffs_config& cfg)
{
	debugf("fs.start: size:%u Kb, offset:0x%X\n", cfg.phys_size / 1024U, cfg.phys_addr);

	// Simple check of the erase count to see if flash looks like it's already been formatted
	spiffs_obj_id dat;
	flashmem_read(&dat, cfg.phys_addr + cfg.log_page_size - sizeof(spiffs_obj_id), sizeof(spiffs_obj_id));
	//debugf("%X", dat);
	bool isFormatted = (dat != spiffs_obj_id(UINT32_MAX));

	if(!isFormatted) {
		debugf("First init file system");
		spiffs_format_internal(cfg);
	}

	if(!tryMount(cfg)) {
		return false;
	}

	if(!isFormatted) {
		spiffs_file fd = SPIFFS_open(&_filesystemStorageHandle, _F("initialize_fs_header.dat"),
									 SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
		uint8_t c{1};
		SPIFFS_write(&_filesystemStorageHandle, fd, &c, 1);
		SPIFFS_fremove(&_filesystemStorageHandle, fd);
		SPIFFS_close(&_filesystemStorageHandle, fd);
	}

	return true;
}

bool initConfig(spiffs_config& cfg, uint32_t phys_addr, uint32_t phys_size)
{
	if(phys_addr == 0) {
		SYSTEM_ERROR("SPIFFS: Start address invalid");
		return false;
	}

	cfg = spiffs_config{
		.hal_read_f = api_spiffs_read,
		.hal_write_f = api_spiffs_write,
		.hal_erase_f = api_spiffs_erase,
		.phys_size = phys_size,
		.phys_addr = phys_addr,
		.phys_erase_block = INTERNAL_FLASH_SECTOR_SIZE,
		.log_block_size = INTERNAL_FLASH_SECTOR_SIZE * 2,
		.log_page_size = LOG_PAGE_SIZE,
	};

	return true;
}

} // namespace

bool spiffs_mount()
{
	spiffs_config cfg = spiffs_get_storage_config();
	return spiffs_mount_manual(cfg.phys_addr, cfg.phys_size);
}

bool spiffs_mount_manual(uint32_t phys_addr, uint32_t phys_size)
{
	spiffs_config cfg;
	if(!initConfig(cfg, phys_addr, phys_size)) {
		return false;
	}
	return spiffs_mount_internal(cfg);
}

void spiffs_unmount()
{
	SPIFFS_unmount(&_filesystemStorageHandle);
}

bool spiffs_format()
{
	auto cfg = spiffs_get_storage_config();
	return spiffs_format_manual(cfg.phys_addr, cfg.phys_size);
}

bool spiffs_format_manual(uint32_t phys_addr, uint32_t phys_size)
{
	spiffs_config cfg;
	if(!initConfig(cfg, phys_addr, phys_size)) {
		return false;
	}
	spiffs_format_internal(cfg);
	return spiffs_mount_internal(cfg);
}
