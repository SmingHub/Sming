/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DataSourceStream.h
 *
 ****/

#pragma once

#include <user_config.h>
#include "Stream.h"
#include "WString.h"

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

/** @defgroup   stream Stream functions
 *  @brief      Data stream classes
 *  @{
*/

///Base class for data source stream
class IDataSourceStream : public Stream
{
public:
	/** @brief  Get the stream type
     *  @retval StreamType The stream type.
     */
	virtual StreamType getStreamType() const
	{
		return eSST_Unknown;
	}

	/** @brief Determine if the stream object contains valid data
	 *  @retval bool true if valid, false if invalid
	 *  @note Where inherited classes are initialised by constructor this method indicates
	 *  whether that was successful or not (e.g. FileStream)
	 */
	virtual bool isValid() const
	{
		return getStreamType() != eSST_Invalid;
	}

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
	int read() override;

	/**
	 * @brief Read a character without advancing the stream pointer
	 * @retval int The character that was read or -1 if none is available
	 */
	int peek() override;

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
	 * From Stream class: We don't write using this stream
	 */
	size_t write(uint8_t charToWrite) override
	{
		return 0;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 *
	 * @deprecated Use `available()` instead
	 */
	int length() SMING_DEPRECATED
	{
		return available();
	}

	/*
	 * @brief Flushes the stream
	 */
	void flush() override
	{
	}

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
	virtual String getName() const
	{
		return nullptr;
	}
};

/** @} */
