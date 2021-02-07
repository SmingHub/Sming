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
#include <IFS/SPIFFS/FileSystem.h>
#include <FileSystem.h>
#include <Storage.h>

namespace
{
Storage::Partition findDefaultPartition()
{
	auto it = Storage::findPartition(Storage::Partition::SubType::Data::spiffs);
	if(!it) {
		debug_e("No SPIFFS partition found");
	}
	return *it;
}

} // namespace

bool spiffs_mount(Storage::Partition partition)
{
	auto fs = new IFS::SPIFFS::FileSystem(partition);
	int err = fs->mount();
	if(err < 0) {
		debug_e("SPIFFS mount failed: %s", fs->getErrorString(err).c_str());
		delete fs;
		return false;
	}

	fileSetFileSystem(fs);
	return true;
}

bool spiffs_mount()
{
	auto part = findDefaultPartition();
	return part ? spiffs_mount(part) : false;
}

void spiffs_unmount()
{
	if(fileSystemType() == IFS::IFileSystem::Type::SPIFFS) {
		fileFreeFileSystem();
	}
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
	spiffs_unmount();
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
