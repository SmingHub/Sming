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
IFS::FileSystem* activeFileSystem;
}

namespace IFS
{
FileSystem* getDefaultFileSystem()
{
	return SmingInternal::activeFileSystem;
}

} // namespace IFS

void fileSetFileSystem(IFS::IFileSystem* fileSystem)
{
	if(SmingInternal::activeFileSystem != fileSystem) {
		delete SmingInternal::activeFileSystem;
		SmingInternal::activeFileSystem = IFS::FileSystem::cast(fileSystem);
	}
}

bool fileMountFileSystem(IFS::IFileSystem* fs)
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

bool fwfs_mount()
{
	auto part = Storage::findDefaultPartition(Storage::Partition::SubType::Data::fwfs);
	return part ? fwfs_mount(part) : false;
}

bool fwfs_mount(Storage::Partition partition)
{
	auto fs = IFS::createFirmwareFilesystem(partition);
	return fileMountFileSystem(fs);
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
