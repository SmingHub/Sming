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

#include "SslConstants.h"
#include "SslConnection.h"

/**
 * @ingroup ssl
 * @brief Ssl Factory class
 * 		  Each SSL adapter must implement at least that class
 * @{
 */

class SslFactory
{
public:
	virtual ~SslFactory()
	{
	}

	/**
	 * @brief Create SSL context that can be used to create new client or server connections
	 * @retval SslContext* return null if the adapter cannot handle SSL. Useful only for dummy SSL adapters
	 */
	virtual SslContext* sslCreateContext() = 0;
};
