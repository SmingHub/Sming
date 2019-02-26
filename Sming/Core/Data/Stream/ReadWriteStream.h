/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ReadWriteStream.h
 *
 ****/

#pragma once

#include "DataSourceStream.h"

/**
  * @brief      Base class for read/write stream
  * @ingroup    stream data
  *
  *  @{
 */

class ReadWriteStream : public IDataSourceStream
{
public:
	/** @brief Write a single character to the stream
	 *  @param charToWrite
	 *  @retval size_t Number of chars written (1 on success, 0 on failure)
	 */
	size_t write(uint8_t charToWrite) override
	{
		return write(&charToWrite, 1);
	}

	/** @brief  Write chars to stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to write
     *  @retval size_t Quantity of chars written to stream
     *  @note Although this is defined in the Print class, ReadWriteStream
     *  uses this as the core output method so descendants are required
     *  to implement it
     */
	virtual size_t write(const uint8_t* buffer, size_t size) = 0;

	/** @brief  Copy data from a source stream
     *  @param  source Stream to read data from
     *  @param  size Quantity of chars to write, determines size of intermediate buffer to use
     *  @retval size_t Quantity of chars actually written, may be less than requested
     */
	virtual size_t copyFrom(IDataSourceStream* source, size_t size);
};

/** @} */
