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
#include <Network/Ssl/Crypto.h>
#include "X509Context.h"

namespace Ssl
{
class BrCertificate : public Ssl::Certificate
{
public:
	BrCertificate(X509Context* context) : context(context)
	{
	}

	bool matchFingerprint(const uint8_t* hash) const override
	{
		return context->matchFingerprint(hash);
	}

	bool matchPki(const uint8_t* hash) const override
	{
		return context->matchPki(hash);
	}

	String getName(DN dn, RDN rdn) const override;

private:
	X509Context* context;
};

} // namespace Ssl
