/**
 * FileSystem.h
 * Provides an IFS FileSystem implementation for SPIFFS.
 *
 * Created on: 21 Jul 2018
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the SPIFFS IFS Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 *
 * This is mostly a straightforward wrapper around SPIFFS, with a few enhancements:
 *
 *	Metadata caching
 *
 *		Metadata can be updated multiple times while a file is open so
 * 		for efficiency it is kept in RAM and only written to SPIFFS on close or flush.
 *
 *	Directory emulation
 *
 *		SPIFFS stores all files in a flat format, so directory behaviour is emulated
 *		including opendir() and readdir() operations. Overall path length is fixed
 *		according to SPIFFS_OBJ_NAME_LEN.
 *
 *  File truncation
 *
 *  	Standard IFS truncate() method allows file size to be reduced.
 *  	This was added to Sming in version 4.
 *
 */

#pragma once

#include <IFS/IFileSystem.h>
#include "FileMeta.h"
#include "../../../../spiffs/src/spiffs.h"
extern "C" {
#include "../../../../spiffs/src/spiffs_nucleus.h"
}

namespace IFS
{
namespace SPIFFS
{
/*
 * Wraps SPIFFS
 */
class FileSystem : public IFileSystem
{
public:
	FileSystem(Storage::Partition partition) : partition(partition)
	{
	}

	~FileSystem();

	int mount() override;
	int getinfo(Info& info) override;
	int setProfiler(IProfiler* profiler) override;
	String getErrorString(int err) override;
	int opendir(const char* path, DirHandle& dir) override;
	int readdir(DirHandle dir, Stat& stat) override;
	int rewinddir(DirHandle dir) override;
	int closedir(DirHandle dir) override;
	int mkdir(const char* path) override;
	int stat(const char* path, Stat* stat) override;
	int fstat(FileHandle file, Stat* stat) override;
	int fsetxattr(FileHandle file, AttributeTag tag, const void* data, size_t size) override;
	int fgetxattr(FileHandle file, AttributeTag tag, void* buffer, size_t size) override;
	int fenumxattr(FileHandle file, AttributeEnumCallback callback, void* buffer, size_t bufsize) override;
	int setxattr(const char* path, AttributeTag tag, const void* data, size_t size) override;
	int getxattr(const char* path, AttributeTag tag, void* buffer, size_t size) override;
	FileHandle open(const char* path, OpenFlags flags) override;
	int close(FileHandle file) override;
	int read(FileHandle file, void* data, size_t size) override;
	int write(FileHandle file, const void* data, size_t size) override;
	file_offset_t lseek(FileHandle file, file_offset_t offset, SeekOrigin origin) override;
	int eof(FileHandle file) override;
	file_offset_t tell(FileHandle file) override;
	int ftruncate(FileHandle file, file_size_t new_size) override;
	int flush(FileHandle file) override;
	int rename(const char* oldpath, const char* newpath) override;
	int remove(const char* path) override;
	int fremove(FileHandle file) override;
	int format() override;
	int check() override;

	/** @brief get the full path of a file from its ID
	 *  @param fileid
	 *  @param buffer
	 *  @retval int error code
	 */
	int getFilePath(FileID fileid, NameBuffer& buffer);

private:
	spiffs* handle()
	{
		return &fs;
	}

	int tryMount(spiffs_config& cfg);

	SpiffsMetaBuffer* initMetaBuffer(FileHandle file);
	SpiffsMetaBuffer* getMetaBuffer(FileHandle file);
	int flushMeta(FileHandle file);

	void touch(FileHandle file)
	{
		auto smb = getMetaBuffer(file);
		if(smb != nullptr) {
			smb->setFileTime(fsGetTimeUTC());
		}
	}

	static s32_t f_read(struct spiffs_t* spiffs, u32_t addr, u32_t size, u8_t* dst);
	static s32_t f_write(struct spiffs_t* spiffs, u32_t addr, u32_t size, u8_t* src);
	static s32_t f_erase(struct spiffs_t* spiffs, u32_t addr, u32_t size);

	static constexpr uint32_t MAX_PARTITION_SIZE{256 * 1024 * 1024};
	static constexpr size_t CACHE_PAGES{8};
	static constexpr size_t LOG_PAGE_SIZE{256};
	static constexpr size_t MIN_BLOCKSIZE{256};
	static constexpr size_t CACHE_PAGE_SIZE{sizeof(spiffs_cache_page) + LOG_PAGE_SIZE};
	static constexpr size_t CACHE_SIZE{sizeof(spiffs_cache) + CACHE_PAGES * CACHE_PAGE_SIZE};

	Storage::Partition partition;
	IProfiler* profiler{nullptr};
	SpiffsMetaBuffer metaCache[SPIFF_FILEDESC_COUNT];
	spiffs fs;
	uint16_t workBuffer[LOG_PAGE_SIZE];
	spiffs_fd fileDescriptors[SPIFF_FILEDESC_COUNT];
	uint8_t cache[CACHE_SIZE];
};

} // namespace SPIFFS
} // namespace IFS
