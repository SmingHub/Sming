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
#include "spiffs/src/spiffs_nucleus.h"

spiffs _filesystemStorageHandle;

#ifndef SPIFF_FILEDESC_COUNT
#define SPIFF_FILEDESC_COUNT 7
#endif

static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[sizeof(spiffs_fd) * SPIFF_FILEDESC_COUNT];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*4];

static s32_t api_spiffs_read(u32_t addr, u32_t size, u8_t *dst)
{
  flashmem_read(dst, addr, size);
  return SPIFFS_OK;
}

static s32_t api_spiffs_write(u32_t addr, u32_t size, u8_t *src)
{
  //debugf("api_spiffs_write");
  flashmem_write(src, addr, size);
  return SPIFFS_OK;
}

static s32_t api_spiffs_erase(u32_t addr, u32_t size)
{
  debugf("api_spiffs_erase");
  u32_t sect_first = flashmem_get_sector_of_address(addr);
  u32_t sect_last = sect_first;
  while( sect_first <= sect_last )
    if( !flashmem_erase_sector( sect_first ++ ) )
      return SPIFFS_ERR_INTERNAL;
  return SPIFFS_OK;
} 

/*******************
The W25Q32BV array is organized into 16,384 programmable pages of 256-bytes each. Up to 256 bytes can be programmed at a time.
Pages can be erased in groups of 16 (4KB sector erase), groups of 128 (32KB block erase), groups of 256 (64KB block erase) or
the entire chip (chip erase). The W25Q32BV has 1,024 erasable sectors and 64 erasable blocks respectively.
The small 4KB sectors allow for greater flexibility in applications that require data and parameter storage.
********************/

bool spiffs_format_internal(spiffs_config *cfg)
{
  if (cfg->phys_addr == 0)
  {
	SYSTEM_ERROR("Can't format file system, wrong address given.");
	return false;
  }

  if (cfg->phys_size == 0)
  {
	SYSTEM_ERROR("Can't format file system, wrong size given.");
	return false;
  }

  uint32_t log_block_count = cfg->phys_size / cfg->log_block_size;
  uint32_t log_block_erased = 0;

  debugf("sect_first: %x, sect_last: %x\n", 
    flashmem_get_sector_of_address(cfg->phys_addr), 
    flashmem_get_sector_of_address(cfg->phys_addr + cfg->phys_size - 1)
  );
  ETS_INTR_LOCK();

  while(log_block_erased < log_block_count)
  {
    uint32_t sector_per_block = cfg->log_block_size / INTERNAL_FLASH_SECTOR_SIZE;
    uint32_t sector_erased = 0;
    while(sector_erased < sector_per_block)
    {
      uint32_t target_sector = flashmem_get_sector_of_address(
        cfg->phys_addr + (log_block_erased * cfg->log_block_size) +
        (sector_erased * INTERNAL_FLASH_SECTOR_SIZE)
      );


      if(!flashmem_erase_sector(target_sector))
      {
        ETS_INTR_UNLOCK();
        return false;
      }
      
      sector_erased++;
    }

    //write erase count to first sector of every logical block
    uint32_t erase_count_addr = cfg->phys_addr + (log_block_erased * cfg->log_block_size);
    erase_count_addr += cfg->log_page_size;
    erase_count_addr -= sizeof(spiffs_obj_id);

    spiffs_obj_id block_erase_count = 0;
    flashmem_write(&block_erase_count, erase_count_addr, sizeof(spiffs_obj_id));

    log_block_erased++;
  }
  debugf("formatted");
  ETS_INTR_UNLOCK();

  return true;
}

static void spiffs_mount_internal(spiffs_config *cfg)
{
  if (cfg->phys_addr == 0)
  {
	  SYSTEM_ERROR("Can't start file system, wrong address");
	  return;
  }

  debugf("fs.start: size:%d Kb, offset:0x%X\n", cfg->phys_size / 1024, cfg->phys_addr);

  cfg->hal_read_f = api_spiffs_read;
  cfg->hal_write_f = api_spiffs_write;
  cfg->hal_erase_f = api_spiffs_erase;
  
  uint32_t dat;
  bool writeFirst = false;
  flashmem_read(&dat, cfg->phys_addr, 4);
  //debugf("%X", dat);

  if (dat == UINT32_MAX)
  {
	  debugf("First init file system");
	  spiffs_format_internal(cfg);
	  writeFirst = true;
  }

  int res = SPIFFS_mount(&_filesystemStorageHandle,
    cfg,
    spiffs_work_buf,
    spiffs_fds,
    sizeof(spiffs_fds),
    spiffs_cache_buf,
    sizeof(spiffs_cache_buf),
    NULL);
  debugf("mount res: %d\n", res);

  if (writeFirst)
  {
	  spiffs_file fd = SPIFFS_open(&_filesystemStorageHandle, "initialize_fs_header.dat", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
	  SPIFFS_write(&_filesystemStorageHandle, fd, (u8_t *)"1", 1);
	  SPIFFS_fremove(&_filesystemStorageHandle, fd);
	  SPIFFS_close(&_filesystemStorageHandle, fd);
  }

  //dat=0;
  //flashmem_read(&dat, cfg.phys_addr, 4);
  //debugf("%X", dat);
}

void spiffs_mount()
{
  spiffs_config cfg = spiffs_get_storage_config();
  spiffs_mount_internal(&cfg);
}

void spiffs_mount_manual(u32_t phys_addr, u32_t phys_size)
{
  spiffs_config cfg = {0};
  cfg.phys_addr = phys_addr;
  cfg.phys_size = phys_size;
  cfg.phys_erase_block = INTERNAL_FLASH_SECTOR_SIZE; // according to datasheet
  cfg.log_block_size = INTERNAL_FLASH_SECTOR_SIZE * 2; // Important to make large
  cfg.log_page_size = LOG_PAGE_SIZE; // as we said
  spiffs_mount_internal(&cfg);
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
  spiffs_mount();
  return true;
}

bool spiffs_format_manual(u32_t phys_addr, u32_t phys_size)
{

  spiffs_unmount();
  spiffs_config cfg = {0};
  cfg.phys_addr = phys_addr;
  cfg.phys_size = phys_size;
  spiffs_format_internal(&cfg);
  spiffs_mount_manual(phys_addr, phys_size);
  return true;
}
