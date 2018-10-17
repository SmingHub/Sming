/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "FileStream.h"

/* FileStream */

FileStream::FileStream()
{
	handle = -1;
	size = -1;
	pos = 0;
}

FileStream::FileStream(const String& filename)
{
	attach(filename, eFO_ReadOnly);
}

bool FileStream::attach(const String& fileName, FileOpenFlags openFlags)
{
	handle = fileOpen(fileName.c_str(), openFlags);
	if(handle == -1) {
		debug_w("File wasn't found: %s", fileName.c_str());
		size = -1;
		pos = 0;
		return false;
	}

	// Get size
	fileSeek(handle, 0, eSO_FileEnd);
	size = fileTell(handle);

	fileSeek(handle, 0, eSO_FileStart);
	pos = 0;

	debug_d("attached file: %s (%d bytes)", fileName.c_str(), size);
	return true;
}

FileStream::~FileStream()
{
	fileClose(handle);
	handle = 0;
	pos = 0;
}

uint16_t FileStream::readMemoryBlock(char* data, int bufSize)
{
	int len = std::min(bufSize, size - pos);
	int available = fileRead(handle, data, len);
	fileSeek(handle, pos, eSO_FileStart); // Don't move cursor now (waiting seek)
	if(available < 0) {
		available = 0;
	}
	return available;
}

size_t FileStream::write(uint8_t charToWrite)
{
	uint8_t tempbuf[1]{charToWrite};
	return write(tempbuf, 1);
}

size_t FileStream::write(const uint8_t* buffer, size_t size)
{
	if(!fileExist())
		return 0;

	fileSeek(handle, 0, eSO_FileEnd);
	return fileWrite(handle, buffer, size);
}

bool FileStream::seek(int len)
{
	if(len < 0)
		return false;

	bool result = fileSeek(handle, len, eSO_CurrentPos) >= 0;
	if(result)
		pos += len;
	return result;
}

bool FileStream::isFinished()
{
	return fileIsEOF(handle);
}

String FileStream::fileName()
{
	spiffs_stat stat;
	fileStats(handle, &stat);
	return String((char*)stat.name);
}

bool FileStream::fileExist()
{
	return size != -1;
}

String FileStream::id()
{
	spiffs_stat stat;
	fileStats(handle, &stat);

#define ETAG_SIZE 16
	char buf[ETAG_SIZE];
	m_snprintf(buf, ETAG_SIZE, _F("00f-%x-%x0-%x"), stat.obj_id, stat.size, strlen((char*)stat.name));

	return String(buf);
}
