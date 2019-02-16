/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_READ_WRITE_STREAM_H_
#define _SMING_CORE_DATA_READ_WRITE_STREAM_H_

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
	size_t write(uint8_t charToWrite) override
	{
		return write(&charToWrite, 1);
	}

	/** @brief  Write chars to stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to write
     *  @retval size_t Quantity of chars written to stream
     */
	size_t write(const uint8_t* buffer, size_t size) override;

	//Use base class documentation
	uint16_t readMemoryBlock(char* data, int bufSize) override;
};

/** @} */
#endif /* _SMING_CORE_DATA_READ_WRITE_STREAM_H_ */
