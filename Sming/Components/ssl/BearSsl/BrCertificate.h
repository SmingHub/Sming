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
#include "X509Context.h"

namespace Ssl
{
class BrCertificate : public Ssl::Certificate, public X509Context
{
public:
	using X509Context::X509Context;

	bool getFingerprint(Fingerprint::Type type, Fingerprint& fingerprint) const override
	{
		return X509Context::getFingerprint(type, fingerprint);
	}

	String getName(DN dn, RDN rdn) const override;
};

} // namespace Ssl
