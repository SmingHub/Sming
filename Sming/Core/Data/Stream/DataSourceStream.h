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
#include <Stream.h>
#include <WString.h>
#include "SeekOrigin.h"
#include <Network/WebConstants.h>

/** @defgroup   stream Stream functions
 *  @brief      Data stream classes
*/

/** @brief  Data stream type
 *  @ingroup constants
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

/**
 * @brief Base class for read-only stream
 * @ingroup stream
 */
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

	size_t readBytes(char* buffer, size_t length) override;

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

	/** @brief Change position in stream
	 *  @param offset
	 *  @param origin
	 *  @retval New position, < 0 on error
	 *  @note This method is implemented by streams which support random seeking,
	 *  such as files and memory streams.
	 */
	virtual int seekFrom(int offset, SeekOrigin origin)
	{
		return -1;
	}

	/** @brief  Move read cursor
	 *  @param  len Relative cursor adjustment
	 *  @retval bool True on success.
	 */
	virtual bool seek(int len)
	{
		return seekFrom(len, SeekOrigin::Current) >= 0;
	}

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
	 * @brief From Stream class: We don't write using this stream
	 * @param charToWrite
	 */
	size_t write(uint8_t charToWrite) override
	{
		(void)charToWrite;
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

	/**
	 * @brief Get MIME type for stream content
	 * @retval MimeType
	 */
	virtual MimeType getMimeType() const
	{
		return ContentType::fromFullFileName(getName(), MIME_UNKNOWN);
	}

	/**
	 * @brief Overrides Stream method for more efficient reading
	 * @note Stream position is updated by this call
	 */
	String readString(size_t maxLen) override;

	/**
	 * @brief Memory-based streams may be able to move content into a String
	 * @param s String object to move data into
	 * @retval bool true on success, false if there's a problem.
	 *
	 * If the operation is not supported by the stream, `s` will be invalidated and false returned.
	 *
	 * Because a String object must have a NUL terminator, this will be appended if there is
	 * sufficient capacity. In this case, the method returns true.
	 *
	 * If there is no capacity to add a NUL terminator, then the final character of stream data
	 * will be replaced with a NUL. The method returns false to indicate this.
	 *
	 */
	virtual bool moveString(String& s)
	{
		s = nullptr;
		return false;
	};
};
