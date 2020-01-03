/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SessionId.h
 *
 ****/

#pragma once

#include "WString.h"
#include <Data/HexString.h>

namespace Ssl
{
/**
 * @brief Manages buffer to store SSL Session ID
 */
class SessionId
{
public:
	const uint8_t* getValue() const
	{
		return reinterpret_cast<const uint8_t*>(value.c_str());
	}

	unsigned getLength() const
	{
		return value.length();
	}

	bool isValid() const
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

	/**
	 * @brief Return a string representation of the session ID
	 */
	String toString() const
	{
		return makeHexString(getValue(), getLength());
	}

	operator String() const
	{
		return toString();
	}

private:
	String value;
};

} // namespace Ssl

/**
 * @deprecated Use Ssl::SessionId instead
 */
typedef Ssl::SessionId SslSessionId SMING_DEPRECATED;

/**
 * @deprecated Use Ssl::SessionId instead
 */
typedef Ssl::SessionId SSLSessionId SMING_DEPRECATED;
