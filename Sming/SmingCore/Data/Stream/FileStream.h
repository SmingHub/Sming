/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_FILESTREAM_H_
#define _SMING_CORE_DATA_FILESTREAM_H_

#include "DataSourceStream.h"
#include "TemplateStream.h"
#include "FileSystem.h"

/**
  * @brief      File stream class
  * @ingroup    stream data
  *
  *  @{
 */

class FileStream : public ReadWriteStream {
public:
	FileStream()
	{}

	/** @brief  Create a file stream
     *  @param  fileName Name of file to open
     */
	FileStream(const String& fileName, FileOpenFlags openFlags = eFO_ReadOnly)
	{
		open(fileName, openFlags);
	}

	FileStream(const spiffs_stat& stat, FileOpenFlags openFlags = eFO_ReadOnly)
	{
		open(stat, openFlags);
	}

	virtual ~FileStream()
	{
		attach(-1, 0);
	}

	// Attach this stream object to an open file handle
	void attach(file_t file, size_t size);

	bool open(const spiffs_stat& stat, FileOpenFlags openFlags = eFO_ReadOnly);
	bool open(const String& fileName, FileOpenFlags openFlags = eFO_ReadOnly);
	void close();

	//Use base class documentation
	virtual StreamType getStreamType() const
	{
		return eSST_File;
	}

	virtual bool isValid() const
	{
		return _handle >= 0;
	}

	virtual size_t write(const uint8_t* buffer, size_t size);

	//Use base class documentation
	virtual size_t readMemoryBlock(char* data, size_t bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	//Use base class documentation
	virtual bool isFinished()
	{
		return fileIsEOF(_handle);
	}

	String fileName() const;

	virtual String name() const
	{
		return fileName();
	}

	bool fileExist() const
	{
		return _handle >= 0;
	}

	/** @brief  Get the offset of cursor from beginning of data
     *  @retval size_t Cursor offset
     */
	inline size_t getPos() const
	{
		return _pos;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available()
	{
		return _size;
	}

	virtual String id() const;

private:
	file_t _handle = -1;
	size_t _pos = 0;
	size_t _size = 0;
};

/** @} */


/**
  * @brief      Template File stream class
  * @ingroup    stream data
  *
  *  @{
 */


class TemplateFileStream : public TemplateStream {
public:
	/** @brief Create a template file stream
     *  @param  templateFileName Template filename
     */
	TemplateFileStream(const String& fileName) : TemplateStream(new FileStream(fileName))
	{}
};


/** @} */


#endif /* _SMING_CORE_DATA_FILESTREAM_H_ */
