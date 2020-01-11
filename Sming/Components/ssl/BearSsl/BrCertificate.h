/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrCertificate.h
 *
 ****/

#pragma once

#include <Network/Ssl/Certificate.h>
#include "X509Name.h"

namespace Ssl
{
class BrCertificate : public Ssl::Certificate
{
public:
	~BrCertificate()
	{
		delete fpCertSha1;
		delete fpCertSha256;
	}

	bool getFingerprint(Fingerprint::Type type, Fingerprint& fingerprint) const override;

	String getName(DN dn, RDN rdn) const override;

	Fingerprint::Cert::Sha1* fpCertSha1 = nullptr;
	Fingerprint::Cert::Sha256* fpCertSha256 = nullptr;
	X509Name issuer;
	X509Name subject;
};

} // namespace Ssl
