/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CertificateImpl.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/Certificate.h>
#include <axtls-8266/ssl/ssl.h>

namespace Ssl
{
class CertificateImpl : public Certificate
{
public:
	CertificateImpl(SSL* ssl) : ssl(ssl)
	{
	}

	~CertificateImpl()
	{
	}

	bool matchFingerprint(const uint8_t* hash) const override
	{
		return (ssl_match_fingerprint(ssl, hash) == 0);
	}

	bool matchPki(const uint8_t* hash) const override
	{
		return (ssl_match_spki_sha256(ssl, hash) == 0);
	}

	const String getName(Name name) const override
	{
		return String(ssl_get_cert_dn(ssl, int(name)));
	}

private:
	SSL* ssl;
};

} // namespace Ssl
