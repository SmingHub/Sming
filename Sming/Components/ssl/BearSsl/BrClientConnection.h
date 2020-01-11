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
class BrClientConnection : public BrConnection, public X509Handler
{
public:
	using BrConnection::BrConnection;

	~BrClientConnection()
	{
		delete certSha1Context;
		delete certSha256Context;
		delete certificate;
	}

	int init();

	const Certificate* getCertificate() const override
	{
		return certificate;
	}

	template <typename T> void freeAndNil(T*& objectPointer)
	{
		delete objectPointer;
		objectPointer = nullptr;
	}

	void freeCertificate() override
	{
		freeAndNil(certificate);
	}

	/* BrConnection */

	br_ssl_engine_context* getEngine() override
	{
		return &clientContext.eng;
	}

	void handshakeComplete() override
	{
		debug_i("Destroy x509");
		freeAndNil(x509);
	}

	/* X509Handler */

	void startCert(uint32_t length) override;
	void appendCertData(const uint8_t* buf, size_t len) override;
	void endCert() override;
	bool endChain() override;

private:
	br_ssl_client_context clientContext;
	BrCertificate* certificate = nullptr;
	X509Context* x509 = nullptr;
	Crypto::Sha1* certSha1Context = nullptr;
	Crypto::Sha256* certSha256Context = nullptr;
};

} // namespace Ssl
