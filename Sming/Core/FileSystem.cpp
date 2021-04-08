/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FileSystem.cpp
 *
 ****/

#include "FileSystem.h"
#include <Storage.h>
#include <debug_progmem.h>

namespace SmingInternal
{
IFS::IFileSystem* activeFileSystem;
}

void fileSetFileSystem(IFS::IFileSystem* fileSystem)
{
	if(SmingInternal::activeFileSystem != fileSystem) {
		delete SmingInternal::activeFileSystem;
		SmingInternal::activeFileSystem = fileSystem;
	}
}

namespace
{
template <typename T> Storage::Partition findDefaultPartition(T subType)
{
	auto part = *Storage::findPartition(subType);
	if(part) {
		debug_i("[%s] Found '%s'", part.typeString().c_str(), part.name().c_str());
	} else {
		debug_e("[%s] No partition found", toString(subType).c_str());
	}
	return part;
}

bool mount(IFS::IFileSystem* fs)
{
	if(fs == nullptr) {
		debug_e("Failed to created filesystem object");
		return false;
	}

	int res = fs->mount();
	debug_i("mount() returned %d (%s)", res, fs->getErrorString(res).c_str());

	if(res < 0) {
		delete fs;
		return false;
	}

	fileSetFileSystem(fs);

	debug_i("File system initialised");
	return true;
}
} // namespace

bool spiffs_mount()
{
	auto part = findDefaultPartition(Storage::Partition::SubType::Data::spiffs);
	return part ? spiffs_mount(part) : false;
}

bool spiffs_mount(Storage::Partition partition)
{
	auto fs = IFS::createSpiffsFilesystem(partition);
	return mount(fs);
}

bool lfs_mount()
{
	auto part = findDefaultPartition(Storage::Partition::SubType::Data::littlefs);
	return part ? lfs_mount(part) : false;
}

bool lfs_mount(Storage::Partition partition)
{
	auto fs = IFS::createLfsFilesystem(partition);
	return mount(fs);
}

bool fwfs_mount()
{
	auto part = findDefaultPartition(Storage::Partition::SubType::Data::fwfs);
	return part ? fwfs_mount(part) : false;
}

bool fwfs_mount(Storage::Partition partition)
{
	auto fs = IFS::createFirmwareFilesystem(partition);
	return mount(fs);
}

bool hyfs_mount()
{
	auto fwfsPart = findDefaultPartition(Storage::Partition::SubType::Data::fwfs);
	auto spiffsPart = findDefaultPartition(Storage::Partition::SubType::Data::spiffs);
	return (fwfsPart && spiffsPart) ? hyfs_mount(fwfsPart, spiffsPart) : false;
}

bool hyfs_mount(Storage::Partition fwfsPartition, Storage::Partition spiffsPartition)
{
	auto fs = IFS::createHybridFilesystem(fwfsPartition, spiffsPartition);
	return mount(fs);
}

Vector<String> fileList()
{
	Vector<String> result;

	DirHandle dir;
	if(fileOpenDir(nullptr, dir) == FS_OK) {
		FileNameStat stat;
		while(fileReadDir(dir, stat) >= 0) {
			result.add(stat.name.buffer);
		}
		fileCloseDir(dir);
	}
	return result;
}

IFS::IFileSystem::Type fileSystemType()
{
	if(SmingInternal::activeFileSystem == nullptr) {
		return IFS::IFileSystem::Type::Unknown;
	}
	IFS::IFileSystem::Info info;
	fileGetSystemInfo(info);
	return info.type;
}
