/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_FILE_STREAM_H_
#define _SMING_CORE_DATA_FILE_STREAM_H_

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
	/** @brief  Create a file stream
     *  @param  fileName Name of file to open
     */
	FileStream();
	FileStream(const String& fileName);
	virtual ~FileStream();

	virtual bool attach(const String& fileName, FileOpenFlags openFlags);
	//Use base class documentation
	virtual StreamType getStreamType()
	{
		return eSST_File;
	}

	virtual size_t write(uint8_t charToWrite);
	virtual size_t write(const uint8_t* buffer, size_t size);

	//Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	//Use base class documentation
	virtual bool isFinished();

	String fileName(); ///< Filename of file stream is attached to
	bool fileExist();  ///< True if file exists

	/** @brief  Get the offset of cursor from beginning of data
     *  @retval int Cursor offset
     */
	inline int getPos()
	{
		return pos;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available()
	{
		return size;
	}

	virtual String id();

private:
	file_t handle;
	int pos;
	int size;
};
/** @} */

/** @} */

#endif /* _SMING_CORE_DATA_FILE_STREAM_H_ */
