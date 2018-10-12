/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_DATA_CHUNKED_STREAM_H_
#define _SMING_CORE_DATA_CHUNKED_STREAM_H_

#include "../StreamTransformer.h"

/**
 * @brief      Data chunked stream class
 * @ingroup    stream data
 *
 *  @{
*/

class ChunkedStream : public StreamTransformer
{
public:
	ChunkedStream(ReadWriteStream* stream, size_t resultSize = 512);

	/**
	 * Encodes a chunk of data
	 * @param uint8_t* source - the incoming data
	 * @param size_t sourceLength -length of the incoming data
	 * @param uint8_t* target - the result data. The pointer must point to an already allocated memory
	 * @param int* targetLength - the length of the result data
	 *
	 * @return the length of the encoded target.
	 */
	int encode(uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength);
};

/** @} */
#endif /* _SMING_CORE_DATA_CHUNKED_STREAM_H_ */
