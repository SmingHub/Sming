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

/** @brief Manages buffer to store SSL Session ID
 */
class SslSessionId
{
public:
	/** @brief May be called even when object is null */
	const uint8_t* getValue()
	{
		return this ? reinterpret_cast<const uint8_t*>(value.c_str()) : nullptr;
	}

	/** @brief May be called even when object is null */
	unsigned getLength()
	{
		return this ? value.length() : 0;
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
