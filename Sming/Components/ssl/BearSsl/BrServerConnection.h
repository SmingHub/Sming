/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrServerConnection.h
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "BrConnection.h"
#include "BrPrivateKey.h"

namespace Ssl
{
class BrServerConnection : public BrConnection
{
public:
	using BrConnection::BrConnection;

	~BrServerConnection()
	{
	}

	int init();

	const Certificate* getCertificate() const override
	{
		return nullptr;
	}

	void freeCertificate() override
	{
	}

	br_ssl_engine_context* getEngine() override
	{
		return &serverContext.eng;
	}

private:
	br_ssl_server_context serverContext;
	br_x509_certificate cert;
	BrPrivateKey key;
};

} // namespace Ssl
