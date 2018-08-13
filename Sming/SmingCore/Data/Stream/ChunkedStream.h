/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_DATA_CHUNKEDSTREAM_H_
#define _SMING_CORE_DATA_CHUNKEDSTREAM_H_

#include "StreamTransformer.h"

/**
 * @brief      Data chunked stream class
 * @ingroup    stream data
 *
 *  @{
*/

class ChunkedStream : public StreamTransformer {
public:
	ChunkedStream(IDataSourceStream* stream, size_t resultSize = 512);
};

/** @} */

#endif /* _SMING_CORE_DATA_CHUNKEDSTREAM_H_ */
