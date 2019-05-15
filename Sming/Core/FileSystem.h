/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
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

#include "spiffs_sming.h"
#include "WVector.h"

class String;

typedef signed short file_t; ///< File handle

/// File open flags
enum FileOpenFlags {
	eFO_ReadOnly = SPIFFS_RDONLY,							  ///< Read only file
	eFO_WriteOnly = SPIFFS_WRONLY,							  ///< Write only file
	eFO_ReadWrite = eFO_ReadOnly | eFO_WriteOnly,			  ///< Read and write file
	eFO_CreateIfNotExist = SPIFFS_CREAT,					  ///< Create new file if file does not exist
	eFO_Append = SPIFFS_APPEND,								  ///< Append to file
	eFO_Truncate = SPIFFS_TRUNC,							  ///< Truncate file
	eFO_CreateNewAlways = eFO_CreateIfNotExist | eFO_Truncate ///< Create new file even if file exists
};

static inline FileOpenFlags operator|(FileOpenFlags lhs, FileOpenFlags rhs)
{
	return (FileOpenFlags)((int)lhs | (int)rhs);
}

/// File seek flags
typedef enum {
	eSO_FileStart = SPIFFS_SEEK_SET,  ///< Start of file
	eSO_CurrentPos = SPIFFS_SEEK_CUR, ///< Current position in file
	eSO_FileEnd = SPIFFS_SEEK_END	 ///< End of file
} SeekOriginFlags;

/** @brief  Open file
 *  @param  name File name
 *  @param  flags Mode to open file
 *  @retval file File ID or negative error code
 */
file_t fileOpen(const String& name, FileOpenFlags flags);

/** @brief  Clode file
 *  @param  file ID of file to open
 *  @note   File ID is returned from fileOpen function
 */
void fileClose(file_t file);

/** @brief  Write to file
 *  @param  file File ID
 *  @param  data Pointer to data to write to file
 *  @param  size Quantity of data elements to write to file
 *  @retval int Quantity of data elements actually written to file or negative error code
 */
int fileWrite(file_t file, const void* data, size_t size);

/** @brief  Read from file
 *  @param  file File ID
 *  @param  data Pointer to data buffer in to which to read data
 *  @param  size Quantity of data elements to read from file
 *  @retval int Quantity of data elements actually read from file or negative error code
 */
int fileRead(file_t file, void* data, size_t size);

/** @brief  Position file cursor
 *  @param  file File ID
 *  @param  offset Quantity of bytes to move cursor
 *  @param  origin Position from where to move cursor
 *  @retval Offset within file or negative error code
 */
int fileSeek(file_t file, int offset, SeekOriginFlags origin);

/** @brief  Check if at end of file
 *  @param  file File ID
 *  @retval bool True if at end of file
 */
bool fileIsEOF(file_t file);

/** @brief  Get position in file
 *  @param  file File ID
 *  @retval int32_t Read / write cursor position
 */
int32_t fileTell(file_t file);

/** @brief  Flush pending writes
 *  @param  file File ID
 *  @retval int Size of last file written or negative error number
 */
int fileFlush(file_t file);

/** @brief  Get last file system error code
 *  @param  fd Not used
 *  @todo   Why does fileLastError have unused fd parameter?
 *  @retval int Error code of last file system operation
 */
int fileLastError(file_t fd);

/** @brief  Clear last file system error
 *  @param  fd Not used
 *  @todo   Why does fileClearLastError have unused fd parameter?
 */
void fileClearLastError(file_t fd);

/** @brief  Create or replace file with defined content
 *  @param  fileName Name of file to create or replace
 *  @param  content Pointer to c-string containing content to populate file with
 *  @param  length (optional) number of characters to write
 *  @retval int Positive value (>= 0) represents the numbers of bytes written
 *  			Negative value (< 0) indicates error
 *  @note   This function creates a new file or replaces an existing file and
 *  		populates the file with the content of a c-string buffer.
            If you do not specify `length`, remember to terminate your c-string buffer with a NUL ('\0').
 */
int fileSetContent(const String& fileName, const char* content, int length = -1);

/** @brief  Create or replace file with defined content
 *  @param  fileName Name of file to create or replace
 *  @param  content String containing content to populate file with
 *  @retval int Positive integer represents the numbers of bytes written.
 *  @retval int Negative integer represents the error code of last file system operation.
 *  @note   This function creates a new file or replaces an existing file and
            populates the file with the content of a string.
 */
int fileSetContent(const String& fileName, const String& content);

/** @brief  Get size of file
 *  @param  fileName Name of file
 *  @retval uint32_t Size of file in bytes
 *  @note	Returns 0 if error occurs
 */
uint32_t fileGetSize(const String& fileName);

/** @brief Truncate (reduce) the size of an open file
 *  @param file Open file handle, must have Write access
 *  @param newSize
 *  @retval int error code
 *  @note In POSIX `ftruncate()` can also make the file bigger, however SPIFFS can only
 *  reduce the file size and will return an error if newSize > fileSize
 */
int fileTruncate(file_t file, size_t newSize);

/** @brief Truncate an open file at the current cursor position
 *  @param file Open file handle, must have Write access
 *  @retval int error code
 */
int fileTruncate(file_t file);

/** @brief Truncate (reduce) the size of a file
 *  @param fileName
 *  @param newSize
 *  @retval int error code
 *  @note In POSIX `truncate()` can also make the file bigger, however SPIFFS can only
 *  reduce the file size and will return an error if newSize > fileSize
 */
int fileTruncate(const String& fileName, size_t newSize);

/** @brief  Rename file
 *  @param  oldName Original name of file to rename
 *  @param  newName New name for file
 *  @retval int error code
 */
int fileRename(const String& oldName, const String& newName);

/** @brief  Get list of files on file system
 *  @retval Vector<String> Vector of strings.
            Each string element contains the name of a file on the file system
 */
Vector<String> fileList();

/** @brief  Read content of a file
 *  @param  fileName Name of file to read from
 *  @retval String String variable in to which to read the file content
 *  @note   After calling this function the content of the file is placed in to a string.
 *  The result will be an invalid String (equates to `false`) if the file could not be read.
 *  If the file exists, but is empty, the result will be an empty string "".
 */
String fileGetContent(const String& fileName);

/** @brief  Read content of a file
 *  @param  fileName Name of file to read from
 *  @param  buffer Pointer to a character buffer in to which to read the file content
 *  @param  bufSize Quantity of bytes to read from file
 *  @retval size_t Quantity of bytes read from file or zero on failure
 *  @note   After calling this function the content of the file is placed in to a c-string
            Ensure there is sufficient space in the buffer for file content
            plus extra trailing null, i.e. at least bufSize + 1
    @note   Returns 0 if the file could not be read
 */
size_t fileGetContent(const String& fileName, char* buffer, size_t bufSize);

/** brief   Get file statistics
 *  @param  name File name
 *  @param  stat Pointer to SPIFFS statistic structure to populate
 *  @retval int -1 on error
 *  @note   Pass a pointer to an instantiated fileStats structure
 *  @todo   Document the return value of fileStats
 */
int fileStats(const String& name, spiffs_stat* stat);

/** brief   Get file statistics
 *  @param  file File ID
 *  @param  stat Pointer to SPIFFS statistic structure to populate
 *  @retval int -1 on error
 *  @note   Pass a pointer to an instantiated fileStats structure
 *  @todo   Document the return value of fileStats
 */
int fileStats(file_t file, spiffs_stat* stat);

/** @brief  Delete file
 *  @param  name Name of file to delete
 *  @retval int error code, 0 on success
 */
int fileDelete(const String& name);

/** @brief  Delete file
 *  @param  file ID of file to delete
 *  @retval int error code, 0 on success
 */
int fileDelete(file_t file);

/** @brief  Check if a file exists on file system
 *  @param  name Name of file to check for
 *  @retval bool True if file exists
 */
bool fileExist(const String& name);

/** @} */
