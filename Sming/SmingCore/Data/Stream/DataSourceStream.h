/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/*
 * 12/8/2018 (mikee47)
 *
 * 	All classes except for IDataSourceStream and ReadWriteStream moved to their own modules.
 * 	This is an abstract base class so callers should not need to see the specific stream
 * 	implementation.
 */

#ifndef _SMING_CORE_DATA_DATASOURCESTREAM_H_
#define _SMING_CORE_DATA_DATASOURCESTREAM_H_

#include <user_config.h>
#include "WString.h"
#include "Stream.h"

/** @brief  Data stream type
 *  @ingroup constants
 *  @{
 */
enum StreamType {
	eSST_Invalid,	///< Stream content not valid
	eSST_Memory,	 ///< Memory data stream
	eSST_File,		 ///< File data stream
	eSST_Template,   ///< Template data stream
	eSST_JsonObject, ///< JSON object data stream
	eSST_User,		 ///< User defined data stream
	eSST_Unknown	 ///< Unknown data stream type
};
/** @} */


/**
  * @brief      Base class for data source stream
  * @ingroup    stream data
  *
  *  @{
 */

class IDataSourceStream : public Stream {
public:
	virtual ~IDataSourceStream()
	{}

	/** @brief  Get the stream type
     *  @retval StreamType The stream type.
     *  @todo   Return value of IDataSourceStream:getStreamType base class function should be of type StreamType, e.g. eSST_User
     */
	virtual StreamType getStreamType() const = 0;

	virtual bool isValid() const
	{
		return getStreamType() != eSST_Invalid;
	}

	/** @brief  Read a block of memory
     *  @param  data Pointer to the data to be read
     *  @param  bufSize Quantity of chars to read
     *  @retval uint16_t Quantity of chars read
     */
	virtual size_t readMemoryBlock(char* data, size_t bufSize) = 0;

	/**
	 * @brief Read one character and moves the stream pointer
	 * @retval int The character that was read or -1 if none is available
	 */
	virtual int read()
	{
		int res = peek();
		if (res >= 0)
			seek(1);
		return res;
	}

	/**
	 * @brief Read a character without advancing the stream pointer
	 * @retval int The character that was read or -1 if none is available
	 */
	virtual int peek()
	{
		char c;
		return (readMemoryBlock(&c, 1) == 1) ? c : -1;
	}

	/** @brief  Move read cursor
	 *  @param  len Relative cursor adjustment
	 *  @retval bool True on success.
	 */
	virtual bool seek(int len) = 0;

	/** @brief  Check if all data has been read
     *  @retval bool True on success.
     */
	virtual bool isFinished() = 0;

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	virtual int available()
	{
		return -1;
	}

	/*
	 * From Stream class
	 */
	virtual size_t write(uint8_t charToWrite)
	{
		return 0;
	}

	/*
	 * @brief Flushes the stream
	 */
	virtual void flush()
	{}

	/**
	 * @brief Returns unique id of the resource.
	 * @retval String the unique id of the stream.
	 */
	virtual String id() const
	{
		return nullptr;
	}

	/**
	 * @brief Returns name of the resource.
	 * @retval String
	 * @note Commonly used to obtain name of file
	 */
	virtual String name() const
	{
		return nullptr;
	}
};

/** @} */



/**
  * @brief      Base class for read/write stream
  * @ingroup    stream data
  *
  *  @{
 */

class ReadWriteStream : public IDataSourceStream {
public:
	virtual ~ReadWriteStream()
	{}

	virtual size_t write(uint8_t charToWrite)
	{
		return write(&charToWrite, 1);
	}

	/** @brief  Write chars to stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to write
     *  @retval size_t Quantity of chars written to stream
     */
	virtual size_t write(const uint8_t* buffer, size_t size) = 0;

	//Use base class documentation
	virtual size_t readMemoryBlock(char* data, size_t bufSize) = 0;
};

/** @} */

#endif /* _SMING_CORE_DATA_DATASOURCESTREAM_H_ */
