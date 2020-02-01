/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spiffs_sming.c
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

static u8_t spiffs_work_buf[LOG_PAGE_SIZE * 2];
static u8_t spiffs_fds[sizeof(spiffs_fd) * SPIFF_FILEDESC_COUNT];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE + 32) * 4];

static s32_t api_spiffs_read(u32_t addr, u32_t size, u8_t* dst)
{
	return (flashmem_read(dst, addr, size) == size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

static s32_t api_spiffs_write(u32_t addr, u32_t size, u8_t* src)
{
	//debugf("api_spiffs_write");
	return (flashmem_write(src, addr, size) == size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

static s32_t api_spiffs_erase(u32_t addr, u32_t size)
{
	debugf("api_spiffs_erase");
	u32_t sect_first = flashmem_get_sector_of_address(addr);
	u32_t sect_last = sect_first;
	while(sect_first <= sect_last) {
		if(!flashmem_erase_sector(sect_first++)) {
			return SPIFFS_ERR_INTERNAL;
		}
	}
	return SPIFFS_OK;
}

static bool tryMount(spiffs_config* cfg)
{
	int res = SPIFFS_mount(&_filesystemStorageHandle, cfg, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds),
						   spiffs_cache_buf, sizeof(spiffs_cache_buf), nullptr);
	debugf("mount res: %d", res);

	return res >= 0;
}

bool spiffs_format_internal(spiffs_config* cfg)
{
	if(cfg->phys_size == 0) {
		SYSTEM_ERROR("Can't format file system, wrong size given.");
		return false;
	}

	spiffs_unmount();
	if(tryMount(cfg)) {
		spiffs_unmount();
	}

	int res = SPIFFS_format(&_filesystemStorageHandle);
	return res >= 0;
}

static bool spiffs_mount_internal(spiffs_config* cfg)
{
	if(cfg->phys_addr == 0) {
		SYSTEM_ERROR("Can't start file system, wrong address");
		return false;
	}

	debugf("fs.start: size:%d Kb, offset:0x%X\n", cfg->phys_size / 1024, cfg->phys_addr);

	cfg->hal_read_f = api_spiffs_read;
	cfg->hal_write_f = api_spiffs_write;
	cfg->hal_erase_f = api_spiffs_erase;

	// Simple check of the erase count to see if flash looks like it's already been formatted
	spiffs_obj_id dat;
	flashmem_read(&dat, cfg->phys_addr + cfg->log_page_size - sizeof(spiffs_obj_id), sizeof(spiffs_obj_id));
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
		spiffs_file fd = SPIFFS_open(&_filesystemStorageHandle, "initialize_fs_header.dat",
									 SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
		SPIFFS_write(&_filesystemStorageHandle, fd, (u8_t*)"1", 1);
		SPIFFS_fremove(&_filesystemStorageHandle, fd);
		SPIFFS_close(&_filesystemStorageHandle, fd);
	}

	return true;
}

bool spiffs_mount()
{
	spiffs_config cfg = spiffs_get_storage_config();
	return spiffs_mount_internal(&cfg);
}

bool spiffs_mount_manual(u32_t phys_addr, u32_t phys_size)
{
	spiffs_config cfg = {0};
	cfg.phys_addr = phys_addr;
	cfg.phys_size = phys_size;
	cfg.phys_erase_block = INTERNAL_FLASH_SECTOR_SIZE;   // according to datasheet
	cfg.log_block_size = INTERNAL_FLASH_SECTOR_SIZE * 2; // Important to make large
	cfg.log_page_size = LOG_PAGE_SIZE;					 // as we said
	return spiffs_mount_internal(&cfg);
}

void spiffs_unmount()
{
	SPIFFS_unmount(&_filesystemStorageHandle);
}

// FS formatting function
bool spiffs_format()
{
	spiffs_unmount();
	spiffs_config cfg = spiffs_get_storage_config();
	spiffs_format_internal(&cfg);
	return spiffs_mount();
}

bool spiffs_format_manual(u32_t phys_addr, u32_t phys_size)
{
	spiffs_unmount();
	spiffs_config cfg = {0};
	cfg.phys_addr = phys_addr;
	cfg.phys_size = phys_size;
	spiffs_format_internal(&cfg);
	return spiffs_mount_manual(phys_addr, phys_size);
}
