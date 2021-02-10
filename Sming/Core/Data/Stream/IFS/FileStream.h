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

#include "../ReadWriteStream.h"
#include "FsBase.h"

namespace IFS
{
/**
 * @brief    File stream class
 * @ingroup  stream data
 */
class FileStream : public FsBase, public ReadWriteStream
{
public:
	using FsBase::FsBase;

	~FileStream()
	{
		close();
	}

	/** @brief Attach this stream object to an open file handle
	 *  @param file
	 *  @param size
	 */
	void attach(File::Handle file, size_t size);

	bool open(const FileStat& stat, File::OpenFlags openFlags = File::ReadOnly);

	/** @brief Open a file and attach this stream object to it
	 *  @param fileName
	 *  @param openFlags
	 *  @retval bool true on success, false on error
	 *  @note call getLastError() to determine cause of failure
	 */
	bool open(const String& fileName, File::OpenFlags openFlags = File::ReadOnly);

	/** @brief Close file
	 */
	void close();

	StreamType getStreamType() const override
	{
		return eSST_File;
	}

	size_t write(const uint8_t* buffer, size_t size) override;

	int read() override
	{
		char c;
		return readBytes(&c, 1) ? static_cast<unsigned char>(c) : -1;
	}

	size_t readBytes(char* buffer, size_t length) override;

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	int seekFrom(int offset, SeekOrigin origin) override;

	bool isFinished() override
	{
		auto fs = getFileSystem();
		return fs == nullptr || fs->eof(handle) != 0;
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
	File::Handle handle{-1};
	size_t pos{0};
	size_t size{0};
};

} // namespace IFS
