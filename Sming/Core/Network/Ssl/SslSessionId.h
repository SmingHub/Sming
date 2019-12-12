/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslSessionId.h
 *
 ****/

#pragma once

#include "ssl/ssl.h"
#include "WString.h"

/**
 * @brief Manages buffer to store SSL Session ID
 * @ingroup ssl
 */
class SslSessionId
{
public:
	const uint8_t* getValue()
	{
		return reinterpret_cast<const uint8_t*>(value.c_str());
	}

	unsigned getLength()
	{
		return value.length();
	}

	bool isValid()
	{
		return getLength() != 0;
	}

	bool assign(const uint8_t* newValue, unsigned newLength)
	{
		if(!value.setLength(newLength)) {
			return false;
		}
		memcpy(value.begin(), newValue, newLength);
		return true;
	}

private:
	String value;
};
