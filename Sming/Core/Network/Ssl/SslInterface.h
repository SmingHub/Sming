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

#include "Interface/SslContext.h"
#include "Interface/SslConnection.h"
#include "Interface/SslExtension.h"
#include "Interface/SslCertificate.h"
#include "Interface/SslConstants.h"

/**
 * @ingroup ssl
 * @brief Interface defining functions that need to be implemented
 * 		  Each SSL adapter must implement at least these two functions
 * @{
 */

/**
 * @brief Create SSL context that can be used to create new client or server connections
 * @retval SslContext* return null if the adapter cannot handle SSL. Useful only for dummy SSL adapters
 */
SslContext* sslCreateContext();

/**
 * @brief Create SSL extension that can be used to set hostname, max fragment size, etc.
 * @see https://tools.ietf.org/html/rfc6066
 * @retval SslExtension* return null if the adapter does not support SSL extensions.
 */
SslExtension* sslCreateExtension();

/** @} */
