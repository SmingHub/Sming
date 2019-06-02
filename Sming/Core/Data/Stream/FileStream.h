/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FileStream.h
 *
 ****/

#pragma once

#include "ReadWriteStream.h"
#include "FileSystem.h"

/**
  * @brief      File stream class
  * @ingroup    stream data
  *
  *  @{
 */

class FileStream : public ReadWriteStream
{
public:
	FileStream()
	{
	}

	/** @brief  Create a file stream
     *  @param  fileName Name of file to open
     */
	FileStream(const String& fileName, FileOpenFlags openFlags = eFO_ReadOnly)
	{
		open(fileName, openFlags);
	}

	~FileStream()
	{
		close();
	}

	/** @brief Attach this stream object to an open file handle
	 *  @param file
	 *  @param size
	 */
	void attach(file_t file, size_t size);

	/** @deprecated Use `open()` instead */
	bool attach(const String& fileName, FileOpenFlags openFlags = eFO_ReadOnly) SMING_DEPRECATED
	{
		return open(fileName, openFlags);
	}

	/** @brief Open a file and attach this stream object to it
	 *  @param fileName
	 *  @param openFlags
	 *  @retval bool true on success, false on error
	 *  @note call getLastError() to determine cause of failure
	 */
	bool open(const String& fileName, FileOpenFlags openFlags = eFO_ReadOnly);

	/** @brief Close file
	 */
	void close();

	//Use base class documentation
	StreamType getStreamType() const override
	{
		return eSST_File;
	}

	size_t write(const uint8_t* buffer, size_t size) override;

	//Use base class documentation
	uint16_t readMemoryBlock(char* data, int bufSize) override;

	/** @brief Change position in file stream
	 *  @param offset
	 *  @param origin
	 *  @retval New position, < 0 on error
	 */
	int seekFrom(int offset, SeekOriginFlags origin);

	//Use base class documentation
	bool seek(int offset) override
	{
		return seekFrom(offset, eSO_CurrentPos) >= 0;
	}

	//Use base class documentation
	bool isFinished() override
	{
		return fileIsEOF(handle);
	}

	/** @brief Filename of file stream is attached to
	 *  @retval String invalid if stream isn't open
	 */
	String fileName() const;

	/** @brief Determine if file exists
	 *  @retval bool true if stream contains valid file
	 */
	bool fileExist() const
	{
		return handle >= 0;
	}

	String getName() const override
	{
		return fileName();
	}

	bool isValid() const override
	{
		return fileExist();
	}

	/** @brief  Get the offset of cursor from beginning of data
     *  @retval size_t Cursor offset
     */
	size_t getPos() const
	{
		return pos;
	}

	/** @brief  Get the total file size
     *  @retval size_t File size
     */
	size_t getSize() const
	{
		return size;
	}

	/**	@brief Return the maximum bytes available to read, from current position
	 * 	@retval int -1 is returned when the size cannot be determined
	 */
	int available() override
	{
		return size - pos;
	}

	String id() const override;

	/** @brief determine if an error occurred during operation
	 *  @retval int filesystem error code
	 */
	int getLastError()
	{
		return lastError;
	}

	/** @brief Reduce the file size
	 *  @param newSize
	 *  @retval bool true on success
	 */
	bool truncate(size_t newSize);

	/** @brief Truncate file at current position
	 *  @retval bool true on success
	 */
	bool truncate()
	{
		return truncate(pos);
	}

private:
	/** @brief Check file operation result and note error code
	 *  @param res result of fileXXX() operation to check
	 *  @retval bool true if operation was successful, false if error occurred
	 */
	bool check(int res)
	{
		if(res >= 0) {
			return true;
		}

		if(lastError >= 0) {
			lastError = res;
		}
		return false;
	}

private:
	file_t handle = -1;
	size_t pos = 0;
	size_t size = 0;
	int lastError = SPIFFS_OK;
};

/** @} */
