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
#include <Storage.h>
extern "C" {
#include "spiffs/src/spiffs_nucleus.h"
}

#define LOG_PAGE_SIZE 256

spiffs _filesystemStorageHandle;

#ifndef SPIFF_FILEDESC_COUNT
#define SPIFF_FILEDESC_COUNT 7
#endif

namespace
{
uint16_t spiffs_work_buf[LOG_PAGE_SIZE];
spiffs_fd spiffs_fds[SPIFF_FILEDESC_COUNT];
uint32_t spiffs_cache_buf[LOG_PAGE_SIZE + 32];

#define GET_DEVICE()                                                                                                   \
	if(fs == nullptr || fs->user_data == nullptr) {                                                                    \
		debug_e("[SPIFFS] NO DEVICE");                                                                                 \
		return 0;                                                                                                      \
	}                                                                                                                  \
	auto device = static_cast<Storage::Device*>(fs->user_data);

s32_t api_spiffs_read(struct spiffs_t* fs, u32_t addr, u32_t size, u8_t* dst)
{
	GET_DEVICE();
	return device->read(addr, dst, size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

s32_t api_spiffs_write(struct spiffs_t* fs, u32_t addr, u32_t size, u8_t* src)
{
	//debugf("api_spiffs_write");
	GET_DEVICE();
	return device->write(addr, src, size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

s32_t api_spiffs_erase(struct spiffs_t* fs, u32_t addr, u32_t size)
{
	debugf("api_spiffs_erase(0x%08x, 0x%08x)", addr, size);
	GET_DEVICE();
	return device->erase_range(addr, size) ? SPIFFS_OK : SPIFFS_ERR_INTERNAL;
}

spiffs_config initConfig(Storage::Partition& partition)
{
	_filesystemStorageHandle.user_data = partition.getDevice();
	return spiffs_config{
		.hal_read_f = api_spiffs_read,
		.hal_write_f = api_spiffs_write,
		.hal_erase_f = api_spiffs_erase,
		.phys_size = partition.size(),
		.phys_addr = partition.address(),
		.phys_erase_block = INTERNAL_FLASH_SECTOR_SIZE,
		.log_block_size = INTERNAL_FLASH_SECTOR_SIZE * 2,
		.log_page_size = LOG_PAGE_SIZE,
	};
}

bool tryMount(Storage::Partition& partition)
{
	auto cfg = initConfig(partition);
	int res = SPIFFS_mount(&_filesystemStorageHandle, const_cast<spiffs_config*>(&cfg),
						   reinterpret_cast<uint8_t*>(spiffs_work_buf), reinterpret_cast<uint8_t*>(spiffs_fds),
						   sizeof(spiffs_fds), spiffs_cache_buf, sizeof(spiffs_cache_buf), nullptr);
	debugf("mount res: %d", res);

	return res >= 0;
}

bool spiffs_format_internal(Storage::Partition& partition)
{
	spiffs_unmount();
	if(tryMount(partition)) {
		spiffs_unmount();
	}

	int res = SPIFFS_format(&_filesystemStorageHandle);
	return res >= 0;
}

bool spiffs_mount_internal(Storage::Partition& partition)
{
	auto cfg = initConfig(partition);
	debugf("fs.start: size:%u Kb, offset:0x%X\n", cfg.phys_size / 1024U, cfg.phys_addr);

	// Simple check of the erase count to see if flash looks like it's already been formatted
	spiffs_obj_id dat{UINT16_MAX};
	partition.read(cfg.log_page_size - sizeof(spiffs_obj_id), &dat, sizeof(dat));
	//debugf("%X", dat);
	bool isFormatted = (dat != UINT16_MAX);

	if(!isFormatted) {
		debugf("First init file system");
		spiffs_format_internal(partition);
	}

	if(!tryMount(partition)) {
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

Storage::Partition findDefaultPartition()
{
	auto it = Storage::findPartition(Storage::Partition::SubType::Data::spiffs);
	if(!it) {
		debug_e("No SPIFFS partition found");
	}
	return *it;
}

} // namespace

bool spiffs_mount()
{
	auto part = findDefaultPartition();
	return part ? spiffs_mount_internal(part) : false;
}

bool spiffs_mount(Storage::Partition partition)
{
	if(!partition.verify(Storage::Partition::SubType::Data::spiffs)) {
		return false;
	}

	return spiffs_mount_internal(partition);
}

void spiffs_unmount()
{
	SPIFFS_unmount(&_filesystemStorageHandle);
}

bool spiffs_format()
{
	auto part = findDefaultPartition();
	if(!part) {
		return false;
	}

	spiffs_format_internal(part);
	return spiffs_mount_internal(part);
}

bool spiffs_format(Storage::Partition& partition)
{
	if(!partition.verify(Storage::Partition::SubType::Data::spiffs)) {
		return false;
	}

	spiffs_format_internal(partition);
	return spiffs_mount_internal(partition);
}
