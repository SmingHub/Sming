/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslInterface.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include <WString.h>

class SslCertificate
{
public:
	/**
    *
    */
	virtual bool matchFingerprint(const uint8_t* hash) const = 0;

	/**
    * @param hash
    * @retval bool true on success, false otherwise
    */
	virtual bool matchPki(const uint8_t* hash) const = 0;

	/**
   * @brief Retrieve an X.509 distinguished name component
   * @param name the desired distinguished name
   * @retval the value for the desired distinguished name
   */
	virtual const String getName(const String& name) const = 0;
	virtual ~SslCertificate()
	{
	}
};
