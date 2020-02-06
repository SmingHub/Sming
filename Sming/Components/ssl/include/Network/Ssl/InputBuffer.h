/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * InputBuffer.h
 *
 ****/

#pragma once

#include <user_config.h>

namespace Ssl
{
/**
 * @brief Wraps a pbuf for reading in chunks
 */
class InputBuffer
{
public:
	InputBuffer(pbuf* buf) : buf(buf)
	{
	}

	size_t available() const
	{
		return buf ? (buf->tot_len - offset) : 0;
	}

	size_t read(uint8_t* buffer, size_t bufSize);

private:
	pbuf* buf;
	uint16_t offset = 0;
};

} // namespace Ssl
