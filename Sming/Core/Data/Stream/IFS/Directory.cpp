/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Directory.cpp
 *
 * @author mikee47 <mike@sillyhouse.net> May 2019
 *
 ****/

#include "Directory.h"

namespace IFS
{
bool Directory::open(const String& dirName)
{
	auto fs = getFileSystem();
	if(fs == nullptr) {
		return false;
	}

	DirHandle dir;
	int err = fs->opendir(dirName, dir);
	if(!check(err)) {
		debug_w("Directory '%s' open error: %s", dirName.c_str(), fs->getErrorString(err).c_str());
		return false;
	}

	close();
	name = dirName ?: "";
	this->dir = dir;

	return true;
}

void Directory::close()
{
	if(dir != nullptr) {
		auto fs = getFileSystem();
		assert(fs != nullptr);
		fs->closedir(dir);
		dir = nullptr;
	}
	lastError = FS_OK;
}

bool Directory::rewind()
{
	auto fs = getFileSystem();
	if(fs == nullptr) {
		return false;
	}

	int err = fs->rewinddir(dir);
	return err == FS_OK;
}

String Directory::getPath() const
{
	String path('/');
	path += name;
	if(name.length() != 0 && name[name.length() - 1] != '/') {
		path += '/';
	}
	return path;
}

String Directory::getParent() const
{
	if(name.length() == 0 || name == "/") {
		return nullptr;
	}
	String path('/');
	int i = name.lastIndexOf('/');
	if(i >= 0) {
		path.concat(name.c_str(), i);
	}
	return path;
}

bool Directory::next()
{
	auto fs = getFileSystem();
	if(fs == nullptr) {
		return false;
	}

	int err = fs->readdir(dir, dirStat);
	if(check(err)) {
		totalSize += dirStat.size;
		++currentIndex;
		return true;
	}

	debug_w("Directory '%s' read error: %s", name.c_str(), getErrorString(err).c_str());

	return false;
}

} // namespace IFS
