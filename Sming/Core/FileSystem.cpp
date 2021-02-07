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
#include <WString.h>
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
