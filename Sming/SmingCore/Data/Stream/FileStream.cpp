/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "FileStream.h"

/* FileStream */

void FileStream::attach(file_t file, size_t size)
{
	close();
	if (file >= 0) {
		_handle = file;
		_size = size;
		fileSeek(_handle, 0, eSO_FileStart);
		_pos = 0;
	}
}

bool FileStream::open(const spiffs_stat& stat, FileOpenFlags openFlags)
{
	file_t file = fileOpen(stat, openFlags);
	if (file < 0)
		return false;

	attach(file, stat.size);
	return true;
}

bool FileStream::open(const String& fileName, FileOpenFlags openFlags)
{
	file_t file = fileOpen(fileName, openFlags);
	if (file < 0) {
		debug_w("File wasn't found: %s", fileName.c_str());
		return false;
	}

	// Get size
	fileSeek(file, 0, eSO_FileEnd);
	int size = fileTell(file);
	attach(file, size);

	debug_d("attached file: %s (%d bytes)", fileName.c_str(), size);
	return true;
}

void FileStream::close()
{
	if (_handle >= 0) {
		fileClose(_handle);
		_handle = -1;
		_size = 0;
		_pos = 0;
	}
}

size_t FileStream::readMemoryBlock(char* data, size_t bufSize)
{
	if (bufSize <= 0 || _pos >= _size)
		return 0;
	size_t len = _size - _pos;
	if (len > (size_t)bufSize)
		len = (size_t)bufSize;
	int available = fileRead(_handle, data, len);
	// Don't move cursor now (waiting seek)
	fileSeek(_handle, _pos, eSO_FileStart);
	return available > 0 ? available : 0;
}

size_t FileStream::write(const uint8_t* buffer, size_t size)
{
	if (!fileExist())
		return 0;

	fileSeek(_handle, 0, eSO_FileEnd);
	return fileWrite(_handle, buffer, size);
}

bool FileStream::seek(int len)
{
	if (len < 0)
		return false;

	bool result = fileSeek(_handle, len, eSO_CurrentPos) >= 0;
	if (result)
		_pos += len;
	return result;
}

String FileStream::fileName() const
{
	spiffs_stat stat;
	fileStats(_handle, &stat);
	return (const char*)stat.name;
}

String FileStream::id() const
{
	spiffs_stat stat;
	fileStats(_handle, &stat);

#define ETAG_SIZE 16
	char buf[ETAG_SIZE];
	m_snprintf(buf, ETAG_SIZE, _F("00f-%x-%x0-%x"), stat.obj_id, stat.size, strlen((const char*)stat.name));

	return buf;
}
