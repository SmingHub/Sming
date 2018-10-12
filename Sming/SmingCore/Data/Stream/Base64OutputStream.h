/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_DATA_BASE64_OUTPUT_STREAM_H_
#define _SMING_CORE_DATA_BASE64_OUTPUT_STREAM_H_

#include "../StreamTransformer.h"
#include "../Services/libb64/cencode.h"

/**
 * @brief      Base64 Stream
 * @ingroup    stream data
 *
 *  @{
*/

class Base64OutputStream : public StreamTransformer
{
public:
	/**
	 * @brief Stream that transforms bytes of data into base64 data stream
	 * @param ReadWriteStream *stream - source stream
	 * @param size_t resultSize - the size of the intermediate buffer.
	 * 							- it will be created once per object, reused multiple times and kept until the end of the object
	 */
	Base64OutputStream(ReadWriteStream* stream, size_t resultSize = 500);

	/**
	 * Encodes a chunk of data into base64. Keeps a state of the progress.
	 * @param uint8_t* source - the incoming data
	 * @param size_t sourceLength -length of the incoming data
	 * @param uint8_t* target - the result data. The pointer must point to an already allocated memory
	 * @param int* targetLength - the length of the result data
	 *
	 * @return the length of the encoded target.
	 */
	int encode(uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength);

	/**
	 * @brief A method that backs up the current state
	 */
	virtual void saveState();

	/**
	 * @brief A method that restores the last backed up state
	 */
	virtual void restoreState();

private:
	base64_encodestate state;
	base64_encodestate lastState;
};

/** @} */
#endif /* _SMING_CORE_DATA_BASE64_OUTPUT_STREAM_H_ */
