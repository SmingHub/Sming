/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrConnection.h
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "BrConnection.h"
#include "X509Context.h"

namespace Ssl
{
class BrClientConnection : public BrConnection
{
public:
	using BrConnection::BrConnection;

	~BrClientConnection()
	{
		delete certificate;
		delete x509Context;
	}

	int init();

	const Certificate* getCertificate() const override;

	void freeCertificate() override
	{
		delete certificate;
		certificate = nullptr;
	}

	/* BrConnection */

	br_ssl_engine_context* getEngine() override
	{
		return &clientContext.eng;
	}

private:
	br_ssl_client_context clientContext;
	X509Context* x509Context = nullptr;
	mutable BrCertificate* certificate = nullptr;
};

} // namespace Ssl
