#include "spiffs_sming.h"

#define LOG_PAGE_SIZE       256

spiffs _filesystemStorageHandle;

static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[32*7]; // sizeof(spiffs_fd) * K
static u8_t spiffs_cache[(LOG_PAGE_SIZE+32)*4];

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

spiffs_config spiffs_get_storage_config()
{
	spiffs_config cfg = {0};
	cfg.phys_addr = ( u32_t )flashmem_get_first_free_block_address();
	if (cfg.phys_addr == 0)
		return cfg;
	cfg.phys_addr &= 0xFFFFF000;  // get the start address of the sector
	cfg.phys_size = INTERNAL_FLASH_SIZE - ( ( u32_t )cfg.phys_addr);
	cfg.phys_erase_block = INTERNAL_FLASH_SECTOR_SIZE; // according to datasheet
	cfg.log_block_size = INTERNAL_FLASH_SECTOR_SIZE * 2; // Important to make large
	cfg.log_page_size = LOG_PAGE_SIZE; // as we said

	return cfg;
}

bool spiffs_format_internal(spiffs_config *cfg)
{
  if (cfg->phys_addr == 0)
  {
	SYSTEM_ERROR("Can't format file system, wrong address");
	return false;
  }

  u32_t sect_first, sect_last;
  sect_first = cfg->phys_addr;
  sect_first = flashmem_get_sector_of_address(sect_first);
  sect_last = cfg->phys_addr + cfg->phys_size;
  sect_last = flashmem_get_sector_of_address(sect_last);
  debugf("sect_first: %x, sect_last: %x\n", sect_first, sect_last);
  ETS_INTR_LOCK();
  int total = sect_last - sect_first;
  int cur = 0;
  int last = -1;
  while( sect_first <= sect_last )
  {
	if(flashmem_erase_sector( sect_first++ ))
	{
		int percent = cur++ * 100 / total;
		if (percent > last)
			debugf("%d%%", percent);
		last = percent;
	}
	else
	{
		ETS_INTR_UNLOCK();
		return false;
	}
  }
  debugf("formated");
  ETS_INTR_UNLOCK();
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
    spiffs_cache,
    sizeof(spiffs_cache),
    NULL);
  debugf("mount res: %d\n", res);

  if (writeFirst)
  {
	  file_t fd = SPIFFS_open(&_filesystemStorageHandle, "initialize_fs_header.dat", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
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

//int spiffs_check( void )
//{
  // ets_wdt_disable();
  // int res = (int)SPIFFS_check(&_filesystemStorageHandle);
  // ets_wdt_enable();
  // return res;
//}

void test_spiffs()
{
  char buf[12] = {0};

  // Surely, I've mounted spiffs before entering here
  
  spiffs_file fd;
  spiffs_stat st = {0};
  SPIFFS_stat(&_filesystemStorageHandle, "my_file.txt", &st);
  if (st.size <= 0)
  {
	  fd = SPIFFS_open(&_filesystemStorageHandle, "my_file.txt", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
	  if (SPIFFS_write(&_filesystemStorageHandle, fd, (u8_t *)"Hello world", 11) < 0)
		  debugf("errno %d\n", SPIFFS_errno(&_filesystemStorageHandle));
	  SPIFFS_close(&_filesystemStorageHandle, fd);
	  debugf("file created");
  }
  else
	  debugf("file %s exist :)", st.name);


  fd = SPIFFS_open(&_filesystemStorageHandle, "my_file.txt", SPIFFS_RDWR, 0);
  if (SPIFFS_read(&_filesystemStorageHandle, fd, (u8_t *)buf, 11) < 0) debugf("errno %d\n", SPIFFS_errno(&_filesystemStorageHandle));
  SPIFFS_close(&_filesystemStorageHandle, fd);

  debugf("--> %s <--\n", buf);
}
