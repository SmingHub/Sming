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

void FileStream::attach(file_t file, size_t size)
{
	close();
	if(file >= 0) {
		handle = file;
		this->size = size;
		fileSeek(handle, 0, SeekOrigin::Start);
		pos = 0;
		debug_d("attached file: '%s' (%u bytes) #0x%08X", fileName().c_str(), size, this);
	}
}

bool FileStream::open(const FileStat& stat, FileOpenFlags openFlags)
{
	lastError = FS_OK;

	file_t file = fileOpen(stat, openFlags);
	if(!check(file)) {
		return false;
	}

	attach(file, stat.size);
	return true;
}

bool FileStream::open(const String& fileName, FileOpenFlags openFlags)
{
	lastError = FS_OK;

	file_t file = fileOpen(fileName, openFlags);
	if(!check(file)) {
		debug_w("File '%s' open error: %s", fileName.c_str(), fileGetErrorString(file).c_str());
		return false;
	}

	// Get size
	int size = fileSeek(file, 0, SeekOrigin::End);
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
	lastError = FS_OK;
}

size_t FileStream::readBytes(char* buffer, size_t length)
{
	if(buffer == nullptr || length == 0 || pos >= size) {
		return 0;
	}

	int available = fileRead(handle, buffer, std::min(size - pos, length));
	if(!check(available)) {
		return 0;
	}

	pos += size_t(available);

	return available;
}

uint16_t FileStream::readMemoryBlock(char* data, int bufSize)
{
	assert(bufSize >= 0);
	size_t startPos = pos;
	size_t count = readBytes(data, bufSize);

	// Move cursor back to start position
	(void)fileSeek(handle, startPos, SeekOrigin::Start);

	return count;
}

size_t FileStream::write(const uint8_t* buffer, size_t size)
{
	if(!fileExist()) {
		return 0;
	}

	if(pos != this->size) {
		int writePos = fileSeek(handle, 0, SeekOrigin::End);
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

int FileStream::seekFrom(int offset, SeekOrigin origin)
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
	FileNameStat stat;
	int res = fileStats(handle, stat);
	return (res < 0) ? nullptr : stat.name.buffer;
}

String FileStream::id() const
{
	FileStat stat;
	int res = fileStats(handle, stat);
	if(res < 0) {
		return nullptr;
	}

#define ETAG_SIZE 16
	char buf[ETAG_SIZE];
	m_snprintf(buf, ETAG_SIZE, _F("00f-%x-%x0-%x"), stat.id, stat.size, stat.name.length);

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
