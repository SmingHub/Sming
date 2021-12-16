/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FileSystem.h
 *
 ****/

/**	@defgroup filesystem File system
 *	@brief	Access file system
 *  @{
 */

#pragma once

#include <IFS/Helpers.h>
#include <IFS/File.h>
#include <IFS/Directory.h>
#include <Spiffs.h>

using file_t = IFS::FileHandle;
using FileHandle = IFS::FileHandle;
using DirHandle = IFS::DirHandle;
using FileOpenFlag = IFS::OpenFlag;
using FileOpenFlags = IFS::OpenFlags;
using FileAttribute = IFS::FileAttribute;
using FileAttributes = IFS::FileAttributes;
using FileStat = IFS::Stat;
using FileNameStat = IFS::NameStat;
constexpr int FS_OK = IFS::FS_OK;

namespace SmingInternal
{
/** @brief Global file system instance
 *
 * Filing system implementations should use helper functions to setup and
 * initialise a filing system. If successful, call fileSetFileSystem() to make
 * it active. This ensures that any active filing system is dismounted destroyed
 * first.
 *
 */
extern IFS::FileSystem* activeFileSystem;

} // namespace SmingInternal

class File : public IFS::File
{
public:
	File() : IFS::File(SmingInternal::activeFileSystem)
	{
	}
};

/**
  * @brief      Directory stream class
  * @ingroup    stream data
 */
class Directory : public IFS::Directory
{
public:
	Directory() : IFS::Directory(SmingInternal::activeFileSystem)
	{
	}
};

/*
 * Boilerplate check for file function wrappers to catch undefined filesystem.
 */
#define CHECK_FS(_method)                                                                                              \
	auto fileSystem = static_cast<IFS::FileSystem*>(SmingInternal::activeFileSystem);                                  \
	if(fileSystem == nullptr) {                                                                                        \
		debug_e("ERROR in %s(): No active file system", __FUNCTION__);                                                 \
		return FileHandle(IFS::Error::NoFileSystem);                                                                   \
	}

/**
 * @brief Get the currently active file system, if any
 * @retval IFS::FileSystem*
 */
inline IFS::FileSystem* getFileSystem()
{
	if(SmingInternal::activeFileSystem == nullptr) {
		debug_e("ERROR: No active file system");
	}
	return SmingInternal::activeFileSystem;
}

/** @brief Sets the currently active file system
 *  @param fileSystem
 *  @note Any existing filing system is freed first.
 *  Typically the filing system implementation has helper functions which
 *  create and initialise the file system to a valid state. The last step
 *  is to call this function to make it active.
 *	Call this function with nullptr to deactivate the filing system.
 */
void fileSetFileSystem(IFS::IFileSystem* fileSystem);

inline void fileFreeFileSystem()
{
	fileSetFileSystem(nullptr);
}

/**
 * @brief Mount a constructed filesystem with debug messages
 */
bool fileMountFileSystem(IFS::IFileSystem* fs);

/**
 * @brief Mount the first available FWFS volume
 * @retval bool true on success
 */
bool fwfs_mount();

/**
 * @brief Mount SPIFFS volume from a specific partition
 * @retval bool true on success
 */
bool fwfs_mount(Storage::Partition partition);

/** @brief  Open file by path
 *  @param  path Full path to file
 *  @param  flags Mode to open file
 *  @retval file File handle or negative error code
 */
template <typename T> inline FileHandle fileOpen(const T& path, FileOpenFlags flags = File::ReadOnly)
{
	CHECK_FS(open)
	return fileSystem->open(path, flags);
}

/** @brief  Clode file
 *  @param  file Handle of file to close
 *  @note   File Handle is returned from fileOpen function
 */
inline int fileClose(FileHandle file)
{
	CHECK_FS(close)
	return fileSystem->close(file);
}

/** @brief  Write to file
 *  @param  file File handle
 *  @param  data Pointer to data to write to file
 *  @param  size Quantity of data elements to write to file
 *  @retval int Quantity of data elements actually written to file or negative error code
 */
inline int fileWrite(FileHandle file, const void* data, size_t size)
{
	CHECK_FS(write);
	return fileSystem->write(file, data, size);
}

/** @brief  Update file modification time
 *  @param  file File handle
 *  @retval int Error code
 */
inline int fileTouch(FileHandle file)
{
	return fileWrite(file, nullptr, 0);
}

/** @brief  Read from file
 *  @param  file File handle
 *  @param  data Pointer to data buffer in to which to read data
 *  @param  size Quantity of data elements to read from file
 *  @retval int Quantity of data elements actually read from file or negative error code
 */
inline int fileRead(FileHandle file, void* data, size_t size)
{
	CHECK_FS(read)
	return fileSystem->read(file, data, size);
}

/** @brief  Position file cursor
 *  @param  file File handle
 *  @param  offset Quantity of bytes to move cursor
 *  @param  origin Position from where to move cursor
 *  @retval int Offset within file or negative error code
 */
inline int fileSeek(FileHandle file, int offset, SeekOrigin origin)
{
	CHECK_FS(seek)
	return fileSystem->lseek(file, offset, origin);
}

/** @brief  Check if at end of file
 *  @param  file File handle
 *  @retval bool true if at end of file
 */
inline bool fileIsEOF(FileHandle file)
{
	auto fileSystem = getFileSystem();
	return fileSystem ? (fileSystem->eof(file) != 0) : true;
}

/** @brief  Get position in file
 *  @param  file File handle
 *  @retval int32_t Read / write cursor position or error code
 */
inline int fileTell(FileHandle file)
{
	CHECK_FS(tell)
	return fileSystem->tell(file);
}

/** @brief  Flush pending writes
 *  @param  file File handle
 *  @retval int Size of last file written or error code
 */
inline int fileFlush(FileHandle file)
{
	CHECK_FS(flush)
	return fileSystem->flush(file);
}

/** @brief get the text for a returned error code
 *  @param err
 *  @retval String
 */
inline String fileGetErrorString(int err)
{
	auto fileSystem = getFileSystem();
	if(fileSystem == nullptr) {
		return IFS::Error::toString(err);
	}
	return fileSystem->getErrorString(err);
}

/** @brief  Create or replace file with defined content
 *  @param  fileName Name of file to create or replace
 *  @param  content Pointer to c-string containing content to populate file with
 *  @param  length Number of characters to write
 *  @retval int Number of bytes transferred or error code
 *  @note   This function creates a new file or replaces an existing file and
 *  		populates the file with the content of a c-string buffer.
 */
template <typename TFileName> inline int fileSetContent(const TFileName& fileName, const char* content, size_t length)
{
	CHECK_FS(setContent)
	return fileSystem->setContent(fileName, content, length);
}

template <typename TFileName, typename TContent> inline int fileSetContent(const TFileName& fileName, TContent content)
{
	CHECK_FS(setContent)
	return fileSystem->setContent(fileName, content);
}

/** @brief  Get size of file
 *  @param  fileName Name of file
 *  @retval uint32_t Size of file in bytes, 0 on error
 */
template <typename TFileName> inline uint32_t fileGetSize(const TFileName& fileName)
{
	auto fileSystem = getFileSystem();
	return fileSystem ? fileSystem->getSize(fileName) : 0;
}

/** @brief Truncate (reduce) the size of an open file
 *  @param file Open file handle, must have Write access
 *  @param newSize
 *  @retval int Error code
 *  @note In POSIX `ftruncate()` can also make the file bigger, however SPIFFS can only
 *  reduce the file size and will return an error if newSize > fileSize
 */
inline int fileTruncate(FileHandle file, size_t newSize)
{
	CHECK_FS(truncate);
	return fileSystem->ftruncate(file, newSize);
}

/** @brief Truncate an open file at the current cursor position
 *  @param file Open file handle, must have Write access
 *  @retval int Error code
 */
inline int fileTruncate(FileHandle file)
{
	CHECK_FS(truncate);
	return fileSystem->ftruncate(file);
}

/** @brief Truncate (reduce) the size of a file
 *  @param fileName
 *  @param newSize
 *  @retval int Error code
 *  @note In POSIX `truncate()` can also make the file bigger, however SPIFFS can only
 *  reduce the file size and will return an error if newSize > fileSize
 */
template <typename TFileName> int fileTruncate(const TFileName& fileName, size_t newSize)
{
	CHECK_FS(truncate);
	return fileSystem->truncate(fileName, newSize);
}

/** @brief  Rename file
 *  @param  oldName Original name of file to rename
 *  @param  newName New name for file
 *  @retval int Error code
 */
inline int fileRename(const char* oldName, const char* newName)
{
	CHECK_FS(rename)
	return fileSystem->rename(oldName, newName);
}

inline int fileRename(const String& oldName, const String& newName)
{
	return fileRename(oldName.c_str(), newName.c_str());
}

/** @brief  Read content of a file
 *  @param  fileName Name of file to read from
 *  @retval String String variable in to which to read the file content
 *  @note   After calling this function the content of the file is placed in to a string.
 *  The result will be an invalid String (equates to `false`) if the file could not be read.
 *  If the file exists, but is empty, the result will be an empty string "".
 */
template <typename TFileName> String fileGetContent(const TFileName& fileName)
{
	auto fileSystem = getFileSystem();
	return fileSystem ? fileSystem->getContent(fileName) : nullptr;
}

/** @brief  Read content of a file
 *  @param  fileName Name of file to read from
 *  @param  buffer Pointer to a character buffer in to which to read the file content
 *  @param  bufSize Quantity of bytes to read from file
 *  @retval size_t Quantity of bytes read from file or zero on failure
 *  @note   After calling this function the content of the file is placed in to a c-string
 *  		Ensure there is sufficient space in the buffer for file content
 *  		plus extra trailing null, i.e. at least bufSize + 1
 *  		Always check the return value!
 *  @note   Returns 0 if the file could not be read
 */
template <typename TFileName> inline size_t fileGetContent(const TFileName& fileName, char* buffer, size_t bufSize)
{
	auto fileSystem = getFileSystem();
	return fileSystem ? fileSystem->getContent(fileName, buffer, bufSize) : 0;
}

template <typename TFileName> inline size_t fileGetContent(const TFileName& fileName, char* buffer)
{
	auto fileSystem = getFileSystem();
	return fileSystem ? fileSystem->getContent(fileName, buffer) : 0;
}

/** @brief  Get file statistics
 *  @param  name File name
 *  @param  stat Pointer to SPIFFS statistic structure to populate
 *  @retval int Error code
 */
inline int fileStats(const char* fileName, FileStat& stat)
{
	CHECK_FS(stat)
	return fileSystem->stat(fileName, &stat);
}

inline int fileStats(const String& fileName, FileStat& stat)
{
	return fileStats(fileName.c_str(), stat);
}

/** brief   Get file statistics
 *  @param  file File handle
 *  @param  stat Pointer to SPIFFS statistic structure to populate
 *  @retval int Error code
 */
inline int fileStats(FileHandle file, FileStat& stat)
{
	CHECK_FS(fstat)
	return fileSystem->fstat(file, &stat);
}

/** @brief  Delete file
 *  @param  name Name of file to delete
 *  @retval int Error code
 */
inline int fileDelete(const char* fileName)
{
	CHECK_FS(remove)
	return fileSystem->remove(fileName);
}

inline int fileDelete(const String& fileName)
{
	return fileDelete(fileName.c_str());
}

/** @brief  Delete file
 *  @param  file handle of file to delete
 *  @retval int Error code
 */
inline int fileDelete(FileHandle file)
{
	CHECK_FS(fremove)
	return fileSystem->fremove(file);
}

/** @brief  Check if a file exists on file system
 *  @param  fileName Full path to file to check for
 *  @retval bool true if file exists
 */
inline bool fileExist(const char* fileName)
{
	CHECK_FS(stat)
	FileStat stat;
	return fileSystem->stat(fileName, &stat) >= 0 && !stat.attr[FileAttribute::Directory];
}

inline bool fileExist(const String& fileName)
{
	return fileExist(fileName.c_str());
}

/** @brief  Check if a directory exists on file system
 *  @param  dirName Full path to directory to check for
 *  @retval bool true if directory exists
 */
inline bool dirExist(const char* dirName)
{
	CHECK_FS(stat)
	FileStat stat;
	return fileSystem->stat(dirName, &stat) >= 0 && stat.attr[FileAttribute::Directory];
}

inline bool dirExist(const String& dirName)
{
	return dirExist(dirName.c_str());
}

/** @brief Open a named directory for reading
 *  @param name Name of directory to open, empty or "/" for root
 *  @param dir  Directory object
 *  @retval int Error code
 *
 */
inline int fileOpenDir(const char* dirName, DirHandle& dir)
{
	CHECK_FS(opendir)
	return fileSystem->opendir(dirName, dir);
}

inline int fileOpenDir(const String& dirName, DirHandle& dir)
{
	return fileOpenDir(dirName.c_str(), dir);
}

inline int fileOpenRootDir(DirHandle& dir)
{
	return fileOpenDir(nullptr, dir);
}

/** @brief close a directory object
 *  @param dir directory to close
 *  @retval int Error code
 */
inline int fileCloseDir(DirHandle dir)
{
	CHECK_FS(closedir)
	return fileSystem->closedir(dir);
}

/** @brief Read a directory entry
 *  @param dir The directory object returned by fileOpenDir()
 *  @param stat The returned information, owned by DirHandle object
 *  @retval int Error code
 */
inline int fileReadDir(DirHandle dir, FileStat& stat)
{
	CHECK_FS(readdir)
	return fileSystem->readdir(dir, stat);
}

/** @brief Rewind to start of directory entries
 *  @param dir The directory object returned by fileOpenDir()
 *  @retval int Error code
 */
inline int fileRewindDir(DirHandle dir)
{
	CHECK_FS(rewinddir)
	return fileSystem->rewinddir(dir);
}

/** @brief Get basic file system information
 *  @retval int Error code
 */
inline int fileGetSystemInfo(IFS::FileSystem::Info& info)
{
	CHECK_FS(getinfo)
	return fileSystem->getinfo(info);
}

/** @brief Get the type of file system currently mounted (if any)
 *  @retval FileSystemType the file system type
 */
IFS::FileSystem::Type fileSystemType();

/** @brief Format the active file system
 *  @retval int Error code
 */
inline int fileSystemFormat()
{
	CHECK_FS(format)
	return fileSystem->format();
}

/** @brief Perform a consistency check/repair on the active file system
 *  @retval int 0 if OK, < 0 unrecoverable errors, > 0 repairs required
 */
inline int fileSystemCheck()
{
	CHECK_FS(check)
	return fileSystem->check();
}

/** @brief  Set access control information
 *  @param  file File handle
 *  @param acl
 *  @retval int Error code
 */
inline int fileSetACL(FileHandle file, const IFS::ACL& acl)
{
	CHECK_FS(setacl)
	return fileSystem->setacl(file, acl);
}

/** @name Set file attributes
 *  @param filename
 *  @param attr
 *  @retval int Error code
 *  @{
 */
inline int fileSetAttr(const char* filename, FileAttributes attr)
{
	CHECK_FS(setattr)
	return fileSystem->setattr(filename, attr);
}

inline int fileSetAttr(const String& filename, FileAttributes attr)
{
	return fileSetAttr(filename.c_str(), attr);
}
/** @} */

/** @brief Set access control information for file
 *  @param file handle to open file, must have write access
 *  @retval int Error code
 *  @note any writes to file will reset this to current time
 */
inline int fileSetTime(FileHandle file, time_t mtime)
{
	CHECK_FS(settime)
	return fileSystem->settime(file, mtime);
}

/** @} */
