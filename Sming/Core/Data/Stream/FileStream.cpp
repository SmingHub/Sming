/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FileStream.cpp
 *
 ****/

#include "FileStream.h"

/* FileStream */

void FileStream::attach(file_t file, size_t size)
{
	close();
	if(file >= 0) {
		handle = file;
		this->size = size;
		fileSeek(handle, 0, eSO_FileStart);
		pos = 0;
		debug_d("attached file: '%s' (%u bytes) #0x%08X", fileName().c_str(), size, this);
	}
}

bool FileStream::open(const String& fileName, FileOpenFlags openFlags)
{
	lastError = SPIFFS_OK;

	file_t file = fileOpen(fileName, openFlags);
	if(!check(file)) {
		debug_w("File wasn't found: %s", fileName.c_str());
		return false;
	}

	// Get size
	int size = fileSeek(file, 0, eSO_FileEnd);
	if(check(size)) {
		attach(file, size);
		return true;
	}

	fileClose(file);
	return false;
}

void FileStream::close()
{
	if(handle >= 0) {
		fileClose(handle);
		handle = -1;
	}
	size = 0;
	pos = 0;
	lastError = SPIFFS_OK;
}

uint16_t FileStream::readMemoryBlock(char* data, int bufSize)
{
	if(data == nullptr || bufSize <= 0 || pos >= size) {
		return 0;
	}

	int available = fileRead(handle, data, std::min(size - pos, size_t(bufSize)));
	check(available);

	// Don't move cursor now (waiting seek)
	fileSeek(handle, pos, eSO_FileStart);

	return available > 0 ? available : 0;
}

size_t FileStream::write(const uint8_t* buffer, size_t size)
{
	if(!fileExist()) {
		return 0;
	}

	if(pos != this->size) {
		int writePos = fileSeek(handle, 0, eSO_FileEnd);
		if(!check(writePos)) {
			return 0;
		}

		pos = size_t(writePos);
	}

	int written = fileWrite(handle, buffer, size);
	if(check(written)) {
		pos += size_t(written);
		this->size = pos;
	}

	return written > 0 ? written : 0;
}

int FileStream::seekFrom(int offset, SeekOriginFlags origin)
{
	// Cannot rely on return value from fileSeek - failure does not mean position hasn't changed
	fileSeek(handle, offset, origin);
	int newpos = fileTell(handle);
	if(check(newpos)) {
		pos = size_t(newpos);
		if(pos > size) {
			size = pos;
		}
	}
	return newpos;
}

String FileStream::fileName() const
{
	spiffs_stat stat;
	fileStats(handle, &stat);
	return String(reinterpret_cast<const char*>(stat.name));
}

String FileStream::id() const
{
	spiffs_stat stat;
	fileStats(handle, &stat);

#define ETAG_SIZE 16
	char buf[ETAG_SIZE];
	m_snprintf(buf, ETAG_SIZE, _F("00f-%x-%x0-%x"), stat.obj_id, stat.size,
			   strlen(reinterpret_cast<const char*>(stat.name)));

	return String(buf);
}

bool FileStream::truncate(size_t newSize)
{
	bool res = check(fileTruncate(handle, newSize));
	if(res) {
		size = newSize;
		if(pos > size) {
			pos = size;
		}
	}
	return res;
}
