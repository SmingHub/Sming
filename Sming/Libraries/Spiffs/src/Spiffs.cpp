/**
 * Spiffs.cpp
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
 */

#include "include/Spiffs.h"
#include "include/IFS/SPIFFS/FileSystem.h"
#include <FileSystem.h>
#include <Storage.h>

namespace IFS
{
FileSystem* createSpiffsFilesystem(Storage::Partition partition)
{
	auto fs = new SPIFFS::FileSystem(partition);
	return FileSystem::cast(fs);
}

} // namespace IFS

bool spiffs_mount()
{
	auto part = Storage::findDefaultPartition(Storage::Partition::SubType::Data::spiffs);
	return part ? spiffs_mount(part) : false;
}

bool spiffs_mount(Storage::Partition partition)
{
	auto fs = IFS::createSpiffsFilesystem(partition);
	return fileMountFileSystem(fs);
}

void spiffs_unmount()
{
	if(fileSystemType() == IFS::IFileSystem::Type::SPIFFS) {
		fileFreeFileSystem();
	}
}

bool hyfs_mount()
{
	auto fwfsPart = Storage::findDefaultPartition(Storage::Partition::SubType::Data::fwfs);
	auto spiffsPart = Storage::findDefaultPartition(Storage::Partition::SubType::Data::spiffs);
	return (fwfsPart && spiffsPart) ? hyfs_mount(fwfsPart, spiffsPart) : false;
}

bool hyfs_mount(Storage::Partition fwfsPartition, Storage::Partition spiffsPartition)
{
	auto ffs = IFS::createSpiffsFilesystem(spiffsPartition);
	auto fs = IFS::createHybridFilesystem(fwfsPartition, ffs);
	return fileMountFileSystem(fs);
}

bool spiffs_format()
{
	if(fileSystemType() != IFS::IFileSystem::Type::SPIFFS) {
		return false;
	}
	return fileSystemFormat() == FS_OK;
}

bool spiffs_format(Storage::Partition& partition)
{
	if(fileSystemType() == IFS::IFileSystem::Type::SPIFFS) {
		fileFreeFileSystem();
	}
	auto fs = new IFS::SPIFFS::FileSystem(partition);
	int err = fs->format();
	if(err < 0) {
		debug_e("SPIFFS format failed: %s", fs->getErrorString(err).c_str());
		delete fs;
		return false;
	}

	fileSetFileSystem(fs);
	return true;
}
