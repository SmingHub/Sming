/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HostFileStream.h
 *
 ****/

// Required for ftruncate()
#define _POSIX_C_SOURCE 200112L

#include "HostFileStream.h"
#include <unistd.h>
#include <fcntl.h>
#include <BitManipulations.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define CHECK(res) check(res, __FUNCTION__)

static int mapFlags(FileOpenFlags flags)
{
	int f = 0;
	if(flags & eFO_Append) {
		f |= O_APPEND;
	}
	if(flags & eFO_CreateIfNotExist) {
		f |= O_CREAT;
	}
	if(flags & eFO_ReadOnly) {
		f |= O_RDONLY;
	}
	if(flags & eFO_Truncate) {
		f |= O_TRUNC;
	}
	if(flags & eFO_WriteOnly) {
		f |= O_WRONLY;
	}
	return f;
}

bool HostFileStream::check(int res, const char* func)
{
	if(res >= 0) {
		return true;
	}

	lastError = errno;

	debug_e("check(%s) = %d", func, lastError);

	return false;
}

bool HostFileStream::open(const String& fileName, FileOpenFlags openFlags)
{
	close();

	handle = ::open(fileName.c_str(), O_BINARY | mapFlags(openFlags), 0644);
	if(!CHECK(handle)) {
		debug_e("Failed to open '%s'", fileName.c_str());
		return false;
	}

	int len = ::lseek(handle, 0, SEEK_END);
	if(len < 0) {
		len = 0;
	} else if(openFlags & eFO_Append) {
		pos = len;
	} else {
		::lseek(handle, 0, SEEK_SET);
	}
	size = len;

	debug_i("Opened file '%s', %u bytes", fileName.c_str(), size);
	filename = fileName;
	return true;
}

void HostFileStream::close()
{
	if(!isValid()) {
		return;
	}

	::close(handle);
	handle = -1;
	size = 0;
	pos = 0;
	filename = nullptr;
	lastError = 0;
}

size_t HostFileStream::write(const uint8_t* buffer, size_t size)
{
	if(!isValid()) {
		return 0;
	}

	::lseek(handle, 0, SEEK_END);

	int res = ::write(handle, buffer, size);
	if(!CHECK(res)) {
		return 0;
	}
	pos += res;
	if(pos > this->size) {
		this->size = pos;
	}
	return res;
}

uint16_t HostFileStream::readMemoryBlock(char* data, int bufSize)
{
	if(!isValid()) {
		return 0;
	}

	int res = ::lseek(handle, pos, SEEK_SET);
	if(!CHECK(res)) {
		return 0;
	}

	res = ::read(handle, data, bufSize);
	if(!CHECK(res)) {
		return 0;
	}

	return res;
}

int HostFileStream::seekFrom(int offset, unsigned origin)
{
	if(!isValid()) {
		return -1;
	}

	if(origin == SEEK_CUR) {
		origin = SEEK_SET;
		offset += pos;
	}
	int newPos = ::lseek(handle, offset, origin);
	if(!CHECK(newPos)) {
		return -1;
	}

	pos = newPos;
	if(newPos > int(size)) {
		size = newPos;
	}

	return true;
}

bool HostFileStream::isFinished()
{
	return pos == size;
}

bool HostFileStream::truncate(size_t newSize)
{
	if(!isValid()) {
		return false;
	}
	if(::ftruncate(handle, newSize) < 0) {
		return false;
	}
	size = newSize;
	if(pos > newSize) {
		pos = newSize;
	}
	return true;
}
