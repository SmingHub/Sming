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
	 * @param stream - source stream
	 * @param resultSize The size of the intermediate buffer, created once per object and reused multiple times
	 */
	Base64OutputStream(IDataSourceStream* stream, size_t resultSize = 500);

	virtual size_t transform(const uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength);

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
