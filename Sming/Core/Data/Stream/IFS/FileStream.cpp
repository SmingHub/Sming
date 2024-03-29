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

namespace IFS
{
void FileStream::attach(FileHandle file, size_t size)
{
	close();
	if(file < 0) {
		return;
	}

	GET_FS()

	handle = file;
	this->size = size;
	fs->lseek(handle, 0, SeekOrigin::Start);
	pos = 0;

	debug_d("attached file: '%s' (%u bytes) #0x%08X", fileName().c_str(), size, this);
}

bool FileStream::open(const String& fileName, OpenFlags openFlags)
{
	GET_FS(false)

	lastError = FS_OK;

	FileHandle file = fs->open(fileName, openFlags);
	if(!check(file)) {
		debug_d("File '%s' open error: %s", fileName.c_str(), fs->getErrorString(file).c_str());
		return false;
	}

	// Get size
	int size = fs->lseek(file, 0, SeekOrigin::End);
	if(check(size)) {
		attach(file, size);
		return true;
	}

	fs->close(file);
	return false;
}

void FileStream::close()
{
	if(handle >= 0) {
		auto fs = getFileSystem();
		assert(fs != nullptr);
		fs->close(handle);
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

	GET_FS(0)

	int available = fs->read(handle, buffer, std::min(size - pos, length));
	if(!check(available)) {
		return 0;
	}

	pos += size_t(available);

	return available;
}

uint16_t FileStream::readMemoryBlock(char* data, int bufSize)
{
	GET_FS(0)

	assert(bufSize >= 0);
	size_t startPos = pos;
	size_t count = readBytes(data, bufSize);

	// Move cursor back to start position
	(void)fs->lseek(handle, startPos, SeekOrigin::Start);
	pos = startPos;

	return count;
}

size_t FileStream::write(const uint8_t* buffer, size_t size)
{
	GET_FS(0)

	if(pos != this->size) {
		int writePos = fs->lseek(handle, 0, SeekOrigin::End);
		if(!check(writePos)) {
			return 0;
		}

		pos = size_t(writePos);
	}

	int written = fs->write(handle, buffer, size);
	if(check(written)) {
		pos += size_t(written);
		this->size = pos;
	}

	return written > 0 ? written : 0;
}

int FileStream::seekFrom(int offset, SeekOrigin origin)
{
	GET_FS(lastError)

	// Cannot rely on return value from fileSeek - failure does not mean position hasn't changed
	fs->lseek(handle, offset, origin);
	int newpos = fs->tell(handle);
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
	auto fs = getFileSystem();
	if(fs == nullptr) {
		return nullptr;
	}

	NameStat stat;
	int res = fs->fstat(handle, stat);
	return (res < 0 || stat.name.length == 0) ? nullptr : stat.name.buffer;
}

MimeType FileStream::getMimeType() const
{
	String name = fileName();
	if(name.endsWith(".gz")) {
		name.remove(name.length() - 3);
	}
	return ContentType::fromFullFileName(name, MIME_UNKNOWN);
}

String FileStream::id() const
{
	auto fs = getFileSystem();
	if(fs == nullptr) {
		return 0;
	}

	Stat stat;
	int res = fs->fstat(handle, stat);
	if(res < 0) {
		return nullptr;
	}

	/*
		Jan 2024. How ETAGs are generated is not specified. Previous implementation was like this:

			m_snprintf(buf, ETAG_SIZE, _F("00f-%x-%x0-%x"), stat.id, stat.size, stat.name.length);

		Issues are:
			- on some architectures compiler warns about differing types %x vs arguments
			- name can change without length being affected; if name changes then file lookup would fail anyway
			- modification timestamp is a good metric, should be incorporated
	*/
	String id;
	id += "00f-";
	id += String(stat.id, HEX);
	id += '-';
	id += String(stat.size, HEX);
	id += '-';
	id += String(stat.mtime, HEX);
	id += '-';
	id += String(stat.name.length, HEX);

	return id;
}

bool FileStream::truncate(size_t newSize)
{
	auto fs = getFileSystem();
	if(fs == nullptr) {
		return 0;
	}

	bool res = check(fs->ftruncate(handle, newSize));
	if(res) {
		size = newSize;
		if(pos > size) {
			pos = size;
		}
	}
	return res;
}

} // namespace IFS
