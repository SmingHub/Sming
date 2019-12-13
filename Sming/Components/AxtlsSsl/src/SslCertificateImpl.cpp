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

#include "SslCertificateImpl.h"

const String SslCertificateImpl::getName(const SslCertificateName& name) const
{
	if(ssl == nullptr) {
		return String();
	}

	return String(ssl_get_cert_dn(ssl, name));
}

bool SslCertificateImpl::matchFingerprint(const uint8_t* hash) const
{
	return (ssl_match_fingerprint(ssl, hash) == 0);
}

bool SslCertificateImpl::matchPki(const uint8_t* hash) const
{
	return (ssl_match_spki_sha256(ssl, hash) == 0);
}
