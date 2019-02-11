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

#include "ssl/ssl.h"
#include "ssl/tls1.h"

#include <functional>
#include "WVector.h"

typedef std::function<bool(SSL* ssl, void* data)> SslValidatorCallback;

bool sslValidateCertificateSha1(SSL* ssl, void* data);

bool sslValidatePublicKeySha256(SSL* ssl, void* data);

struct SSLValidator {
	SslValidatorCallback callback;
	void* data; ///< Fingerprint
};

class SSLValidatorList : private Vector<SSLValidator>
{
public:
	~SSLValidatorList()
	{
		clear();
	}

	void add(SslValidatorCallback callback, void* data)
	{
		Vector::add(SSLValidator{callback, data});
	}

	bool validate(SSL* ssl);

	void clear();
};

/** @} */
#endif /* _SMING_CORE_SSLVALIDATOR_H_ */
