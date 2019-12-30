/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * InputBuffer.cpp
 *
 ****/

#include <Network/Ssl/InputBuffer.h>

namespace Ssl
{
size_t InputBuffer::read(uint8_t* buffer, size_t bufSize)
{
	if(buf == nullptr) {
		return 0;
	}

	unsigned len = pbuf_copy_partial(buf, buffer, bufSize, offset);
	offset += len;

#ifdef SSL_DEBUG
	if(len < bufSize) {
		debug_d("SSL read input: Bytes needed: %d, Bytes read: %u", bufSize, len);
	}
#endif

	return len;
}

} // namespace Ssl
