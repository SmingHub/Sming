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
#include "WString.h"

file_t fileOpen(const String& name, FileOpenFlags flags)
{
	// Special fix to prevent known spifFS bug: manual delete file
	if((flags & eFO_CreateNewAlways) == eFO_CreateNewAlways) {
		if(fileExist(name)) {
			fileDelete(name);
		}
		flags = (FileOpenFlags)((int)flags & ~eFO_Truncate);
	}

	int res = SPIFFS_open(&_filesystemStorageHandle, name.c_str(), (spiffs_flags)flags, 0);
	if(res < 0) {
		debugf("open errno %d\n", SPIFFS_errno(&_filesystemStorageHandle));
	}

	return res;
}

void fileClose(file_t file)
{
	SPIFFS_close(&_filesystemStorageHandle, file);
}

int fileWrite(file_t file, const void* data, size_t size)
{
	int res = SPIFFS_write(&_filesystemStorageHandle, file, (void*)data, size);
	if(res < 0) {
		debugf("write errno %d\n", SPIFFS_errno(&_filesystemStorageHandle));
	}
	return res;
}

int fileRead(file_t file, void* data, size_t size)
{
	int res = SPIFFS_read(&_filesystemStorageHandle, file, data, size);
	if(res < 0) {
		debugf("read errno %d\n", SPIFFS_errno(&_filesystemStorageHandle));
	}
	return res;
}

int fileSeek(file_t file, int offset, SeekOriginFlags origin)
{
	return SPIFFS_lseek(&_filesystemStorageHandle, file, offset, origin);
}

bool fileIsEOF(file_t file)
{
	return SPIFFS_eof(&_filesystemStorageHandle, file) != 0;
}

int32_t fileTell(file_t file)
{
	return SPIFFS_tell(&_filesystemStorageHandle, file);
}

int fileFlush(file_t file)
{
	return SPIFFS_fflush(&_filesystemStorageHandle, file);
}

int fileStats(const String& name, spiffs_stat* stat)
{
	return SPIFFS_stat(&_filesystemStorageHandle, name.c_str(), stat);
}

int fileStats(file_t file, spiffs_stat* stat)
{
	return SPIFFS_fstat(&_filesystemStorageHandle, file, stat);
}

int fileDelete(const String& name)
{
	return SPIFFS_remove(&_filesystemStorageHandle, name.c_str());
}

int fileDelete(file_t file)
{
	return SPIFFS_fremove(&_filesystemStorageHandle, file);
}

bool fileExist(const String& name)
{
	spiffs_stat stat = {0};
	if(fileStats(name.c_str(), &stat) < 0) {
		return false;
	}
	return stat.name[0] != '\0';
}

int fileLastError(file_t fd)
{
	return SPIFFS_errno(&_filesystemStorageHandle);
}

void fileClearLastError(file_t fd)
{
	SPIFFS_clearerr(&_filesystemStorageHandle);
}

int fileSetContent(const String& fileName, const String& content)
{
	return fileSetContent(fileName, content.c_str(), content.length());
}

int fileSetContent(const String& fileName, const char* content, int length)
{
	file_t file = fileOpen(fileName.c_str(), eFO_CreateNewAlways | eFO_WriteOnly);
	if(file < 0) {
		return file;
	}
	if(length < 0) {
		length = strlen(content);
	}
	int res = fileWrite(file, content, length);
	fileClose(file);
	return res;
}

uint32_t fileGetSize(const String& fileName)
{
	file_t file = fileOpen(fileName.c_str(), eFO_ReadOnly);
	int size = fileSeek(file, 0, eSO_FileEnd);
	fileClose(file);
	return (size < 0) ? 0 : size;
}

int fileRename(const String& oldName, const String& newName)
{
	return SPIFFS_rename(&_filesystemStorageHandle, oldName.c_str(), newName.c_str());
}

Vector<String> fileList()
{
	Vector<String> result;
	spiffs_DIR d;
	spiffs_dirent info;

	SPIFFS_opendir(&_filesystemStorageHandle, "/", &d);
	while(true) {
		if(!SPIFFS_readdir(&d, &info)) {
			break;
		}
		result.add(String((char*)info.name));
	}
	SPIFFS_closedir(&d);
	return result;
}

String fileGetContent(const String& fileName)
{
	String res;
	file_t file = fileOpen(fileName.c_str(), eFO_ReadOnly);
	int size = fileSeek(file, 0, eSO_FileEnd);
	if(size == 0) {
		res = ""; // Valid String, file is empty
	} else if(size > 0) {
		fileSeek(file, 0, eSO_FileStart);
		res.setLength(size);
		if(fileRead(file, res.begin(), res.length()) != size) {
			res = nullptr; // read failed, invalidate String
		}
	}
	fileClose(file);
	return res;
}

size_t fileGetContent(const String& fileName, char* buffer, size_t bufSize)
{
	if(buffer == nullptr || bufSize == 0) {
		return 0;
	}

	file_t file = fileOpen(fileName.c_str(), eFO_ReadOnly);
	int size = fileSeek(file, 0, eSO_FileEnd);
	if(size <= 0 || bufSize <= size_t(size)) {
		size = 0;
	} else {
		fileSeek(file, 0, eSO_FileStart);
		if(fileRead(file, buffer, size) != size) {
			size = 0; // Error
		}
	}
	fileClose(file);
	buffer[size] = '\0';
	return size;
}

int fileTruncate(file_t file, size_t newSize)
{
	return SPIFFS_ftruncate(&_filesystemStorageHandle, file, newSize);
}

int fileTruncate(file_t file)
{
	int pos = fileTell(file);
	return (pos < 0) ? pos : fileTruncate(file, pos);
}

int fileTruncate(const String& fileName, size_t newSize)
{
	file_t file = fileOpen(fileName, eFO_WriteOnly);
	if(file < 0) {
		return file;
	}

	int res = fileTruncate(file, newSize);
	fileClose(file);
	return res;
}
