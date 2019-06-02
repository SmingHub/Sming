/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * GdbFileStream.cpp
 *
 ****/

#include "GdbFileStream.h"
#include <gdb/gdb_syscall.h>

/* GdbFileStream */

bool GdbFileStream::open(const String& fileName, FileOpenFlags openFlags)
{
	lastError = 0;

	int flags = 0;
	if((openFlags & eFO_ReadWrite) == eFO_ReadWrite) {
		flags = O_RDWR;
	} else if(openFlags & eFO_WriteOnly) {
		flags = O_WRONLY;
	} else {
		flags = O_RDONLY;
	}
	if(openFlags & eFO_CreateIfNotExist) {
		flags |= O_CREAT;
	}
	if(openFlags & eFO_Truncate) {
		flags |= O_TRUNC;
	}

	int fd = gdb_syscall_open(fileName.c_str(), flags, S_IRWXU);
	if(!check(fd)) {
		debug_w("File wasn't found: %s", fileName.c_str());
		return false;
	}

	// Get size
	int size = gdb_syscall_lseek(fd, 0, SEEK_END);
	if(check(size)) {
		close();
		if(fd >= 0) {
			handle = fd;
			this->size = size;
			gdb_syscall_lseek(fd, 0, SEEK_SET);
			pos = 0;
			debug_d("opened file: '%s' (%u bytes) #0x%08X", fileName.c_str(), size, this);
		}
		return true;
	}

	gdb_syscall_close(fd);
	return false;
}

void GdbFileStream::close()
{
	if(handle >= 0) {
		gdb_syscall_close(handle);
		handle = -1;
	}
	size = 0;
	pos = 0;
	lastError = 0;
}

uint16_t GdbFileStream::readMemoryBlock(char* data, int bufSize)
{
	if(data == nullptr || bufSize <= 0 || pos >= size) {
		return 0;
	}

	int available = gdb_syscall_read(handle, data, std::min(size - pos, size_t(bufSize)));
	check(available);

	// Don't move cursor now (waiting seek)
	gdb_syscall_lseek(handle, pos, SEEK_SET);

	return available > 0 ? available : 0;
}

size_t GdbFileStream::write(const uint8_t* buffer, size_t size)
{
	if(!fileExist()) {
		return 0;
	}

	int writePos = gdb_syscall_lseek(handle, 0, SEEK_END);
	if(!check(writePos)) {
		return 0;
	}

	pos = size_t(writePos);

	int written = gdb_syscall_write(handle, buffer, size);
	if(check(written)) {
		pos += size_t(written);
		if(pos > this->size) {
			this->size = pos;
		}
	}

	return written;
}

bool GdbFileStream::seek(int len)
{
	int newpos = gdb_syscall_lseek(handle, len, SEEK_CUR);
	if(!check(newpos)) {
		return false;
	}

	pos = size_t(newpos);
	if(pos > size) {
		size = pos;
	}

	return true;
}

String GdbFileStream::id() const
{
	gdb_stat_t stat;
	gdb_syscall_fstat(handle, &stat);

#define ETAG_SIZE 16
	char buf[ETAG_SIZE];
	m_snprintf(buf, ETAG_SIZE, _F("00f-%x-%x0-%x"), stat.st_ino, stat.st_size, fileName.length());

	return String(buf);
}
