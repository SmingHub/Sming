/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

/** @defgroup tcp TCP
 *  @ingroup networking
 *  @{
 */

#ifndef _SMING_CORE_SSLVALIDATOR_H_
#define _SMING_CORE_SSLVALIDATOR_H_

#ifdef ENABLE_SSL

#include "ssl/ssl.h"
#include "ssl/tls1.h"

#include <functional>

typedef std::function<bool(const SSL* ssl, const void* data)> SslValidatorCallback;

bool sslValidateCertificateSha1(const SSL* ssl, const void* data);

bool sslValidatePublicKeySha256(const SSL* ssl, const void* data);

#endif /* ENABLE_SSL */

/** @} */
#endif /* _SMING_CORE_SSLVALIDATOR_H_ */
