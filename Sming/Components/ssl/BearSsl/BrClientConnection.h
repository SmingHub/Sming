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
#include "BrPublicKey.h"
#include "X509Context.h"
#include "X509Decoder.h"

namespace Ssl
{
class BrClientConnection : public BrConnection, public X509Handler
{
public:
	BrClientConnection(Context& context, tcp_pcb* tcp) : BrConnection(context, tcp), x509(*this)
	{
	}

	int init();

	const Certificate* getCertificate() const override
	{
		return certificate.get();
	}

	void freeCertificate() override
	{
		certificate.reset();
	}

	/* BrConnection */

	br_ssl_engine_context* getEngine() override
	{
		return &clientContext.eng;
	}

	/* X509Handler */

	virtual void startChain(const char* serverName) override
	{
	}

	void startCert(uint32_t length) override;
	void appendCertData(const uint8_t* buf, size_t len) override;
	void endCert() override;
	bool endChain() override;

	const br_x509_pkey* getPublicKey() override
	{
		return publicKey;
	}

private:
	br_ssl_client_context clientContext;
	X509Context x509;
	BrPublicKey publicKey;
	std::unique_ptr<X509Decoder> x509Decoder;
	std::unique_ptr<BrCertificate> certificate;
	std::unique_ptr<Crypto::Sha1> certSha1Context;
	std::unique_ptr<Crypto::Sha256> certSha256Context;
};

} // namespace Ssl
