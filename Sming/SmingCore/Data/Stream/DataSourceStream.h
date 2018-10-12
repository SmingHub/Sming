/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_DATA_SOURCE_STREAM_H_
#define _SMING_CORE_DATA_DATA_SOURCE_STREAM_H_

#include <user_config.h>
#include "Stream.h"
#include "WString.h"

/** @brief  Data stream type
 *  @ingroup constants
 *  @{
 */
enum StreamType {
	eSST_Memory,	   ///< Memory data stream
	eSST_File,		   ///< File data stream
	eSST_TemplateFile, ///< Template file data stream
	eSST_JsonObject,   ///< JSON object data stream
	eSST_User,		   ///< User defined data stream
	eSST_Unknown	   ///< Unknown data stream type
};
/** @} */

/** @defgroup   stream Stream functions
 *  @brief      Data stream classes
 *  @{
*/

///Base class for data source stream
class IDataSourceStream : public Stream
{
public:
	virtual ~IDataSourceStream()
	{
	}

	/** @brief  Get the stream type
     *  @retval StreamType The stream type.
     *  @todo   Return value of IDataSourceStream:getStreamType base class function should be of type StreamType, e.g. eSST_User
     */
	virtual StreamType getStreamType() = 0;

	/** @brief  Read a block of memory
     *  @param  data Pointer to the data to be read
     *  @param  bufSize Quantity of chars to read
     *  @retval uint16_t Quantity of chars read
     *  @todo   Should IDataSourceStream::readMemoryBlock return same data type as its bufSize param?
     */
	virtual uint16_t readMemoryBlock(char* data, int bufSize) = 0;

	/**
	 * @brief Read one character and moves the stream pointer
	 * @retval The character that was read or -1 if none is available
	 */
	virtual int read();

	/**
	 * @brief Read a character without advancing the stream pointer
	 * @retval The character that was read or -1 if none is available
	 */
	virtual int peek();

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

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 *
	 * @deprecated This method is deprecated and will be removed in the coming versions.
	 * 			   Please, use available() instead.
	 */
	int length()
	{
		return available();
	}

	/*
	 * @brief Flushes the stream
	 */
	virtual void flush()
	{
	}

	/**
	 * @brief Returns unique id of the resource.
	 * @retval String the unique id of the stream.
	 */
	virtual String id()
	{
		return String();
	}
};

/** @} */
#endif /* _SMING_CORE_DATA_DATA_SOURCE_STREAM_H_ */
