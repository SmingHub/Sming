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

	bool getFingerprint(Fingerprint::Cert::Sha1& fingerprint) const override
	{
		return X509Context::getFingerprint(fingerprint);
	}

	bool getFingerprint(Fingerprint::Cert::Sha256& fingerprint) const override
	{
		return X509Context::getFingerprint(fingerprint);
	}

	//	// There is no easy easy way to obtain this.
	//	bool getFingerprint(Fingerprint::Pki::Sha256& fingerprint) const
	//	{
	//		return false;
	//	}

	String getName(DN dn, RDN rdn) const override;
};

} // namespace Ssl
