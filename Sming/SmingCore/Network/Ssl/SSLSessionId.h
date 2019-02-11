/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_NETWORK_SSLSESSIONID_H_
#define SMINGCORE_NETWORK_SSLSESSIONID_H_

#include "ssl/ssl.h"

/** @brief Manages buffer to store SSL Session ID
 */
struct SSLSessionId {
	uint8_t* value = nullptr;
	unsigned length = 0;

	~SSLSessionId()
	{
		free();
	}

	/** @brief May be called even when SSLSessionId is nullptr */
	uint8_t* getValue()
	{
		return this ? value : nullptr;
	}

	/** @brief May be called even when SSLSessionId is nullptr */
	unsigned getLength()
	{
		return this ? length : 0;
	}

	bool isValid()
	{
		return (this != nullptr) && (value != nullptr) && (length != 0);
	}

	bool assign(const uint8_t* newValue, unsigned newLength)
	{
		assert(newLength <= SSL_SESSION_ID_SIZE);
		if(value == nullptr) {
			value = new uint8_t[SSL_SESSION_ID_SIZE];
			if(value == nullptr) {
				return false;
			}
		}
		memcpy(value, newValue, newLength);
		length = newLength;
		return true;
	}

	void free()
	{
		delete[] value;
		value = nullptr;
		length = 0;
	}
};

#endif /* SMINGCORE_NETWORK_SSLSESSIONID_H_ */
