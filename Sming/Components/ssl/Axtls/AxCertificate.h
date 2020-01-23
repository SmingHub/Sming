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

	template <class FP> static bool copy(FP& fp, const uint8_t* src)
	{
		if(src == nullptr) {
			fp = {};
			return false;
		}

		memcpy(fp.hash.data(), src, fp.hash.size());
		return true;
	};

	bool getFingerprint(Fingerprint::Type type, Fingerprint& fingerprint) const override
	{
		if(ssl->x509_ctx == nullptr) {
			return false;
		}

		switch(type) {
		case Fingerprint::Type::CertSha1:
			return copy(fingerprint.cert.sha1, ssl->x509_ctx->fingerprint);

		case Fingerprint::Type::PkiSha256:
			return copy(fingerprint.pki.sha256, ssl->x509_ctx->spki_sha256);

		default:
			return false;
		}
	}

	String getName(DN dn, RDN rdn) const override;

private:
	SSL* ssl;
};

} // namespace Ssl
