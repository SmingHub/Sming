/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxCertificate.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/Certificate.h>
#include <axtls-8266/ssl/ssl.h>

namespace Ssl
{
class AxCertificate : public Certificate
{
public:
	AxCertificate(SSL* ssl) : ssl(ssl)
	{
		ssl->can_free_certificates = false;
	}

	~AxCertificate()
	{
		ssl->can_free_certificates = true;
	}

	bool getFingerprint(Fingerprint::Cert::Sha1& fingerprint) const override
	{
		return ssl->x509_ctx ? fingerprint.hash.copy(ssl->x509_ctx->fingerprint) : false;
	}

	bool getFingerprint(Fingerprint::Pki::Sha256& fingerprint) const override
	{
		return ssl->x509_ctx ? fingerprint.hash.copy(ssl->x509_ctx->spki_sha256) : false;
	}

	String getName(DN dn, RDN rdn) const override;

private:
	SSL* ssl;
};

} // namespace Ssl
