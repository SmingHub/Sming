/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HostFileStream.h
 *
 ****/

#pragma once

#include <Data/Stream/ReadWriteStream.h>
#include <FileSystem.h>

/**
  * @brief      Host File stream class
  * @ingroup    stream data
  *
  *  @{
 */

class HostFileStream : public ReadWriteStream
{
public:
	HostFileStream()
	{
	}

	/** @brief  Create a file stream
     *  @param  fileName Name of file to open
     *  @param	openFlags
     */
	HostFileStream(const String& fileName, FileOpenFlags openFlags = eFO_ReadOnly)
	{
		open(fileName, openFlags);
	}

	~HostFileStream()
	{
		close();
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

	StreamType getStreamType() const override
	{
		return eSST_File;
	}

	size_t write(const uint8_t* buffer, size_t size) override;

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	int seekFrom(int offset, unsigned origin) override;

	bool isFinished() override;

	String getName() const override
	{
		return filename;
	}

	bool isValid() const override
	{
		return handle >= 0;
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
	bool check(int res, const char* func);

private:
	int handle = -1;
	String filename;
	size_t pos = 0;
	size_t size = 0;
	int lastError = 0;
};

/** @} */
