/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrCertificate.h
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <Network/Ssl/Certificate.h>
#include "X509Name.h"
#include <memory>

namespace Ssl
{
class BrCertificate : public Ssl::Certificate
{
public:
	bool getFingerprint(Fingerprint::Type type, Fingerprint& fingerprint) const override;

	String getName(DN dn, RDN rdn) const override;

	std::unique_ptr<Fingerprint::Cert::Sha1> fpCertSha1;
	std::unique_ptr<Fingerprint::Cert::Sha256> fpCertSha256;
	X509Name issuer;
	X509Name subject;
};

} // namespace Ssl
