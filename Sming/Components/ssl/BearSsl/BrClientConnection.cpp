/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrClientConnection.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

/*
 */
#include <SslDebug.h>
#include "BrClientConnection.h"
#include <Network/Ssl/Session.h>

namespace Ssl
{
int BrClientConnection::init()
{
	br_ssl_client_zero(&clientContext);

	// Use Mono-directional buffer size according to requested max. fragment size
	size_t bufSize = maxBufferSizeToBytes(context.session.maxBufferSize);
	if(bufSize == 0) {
		bufSize = 4096;
	}
	int err = BrConnection::init(bufSize, false);
	if(err < 0) {
		return err;
	}

	br_ssl_client_set_default_rsapub(&clientContext);

	// X509 verification
	delete x509;
	x509 = new X509Context(*this);
	br_ssl_engine_set_x509(getEngine(), *x509);

	br_ssl_client_reset(&clientContext, context.session.hostName.c_str(), 0);

	return startHandshake();
}

template <class HashCtx> void resetHash(HashCtx*& ctx, bool create)
{
	delete ctx;
	if(create) {
		ctx = new HashCtx;
	} else {
		ctx = nullptr;
	}
}

void BrClientConnection::startCert(uint32_t length)
{
	if(x509->count() != 0) {
		return;
	}

	auto& types = context.session.validators.fingerprintTypes;
	resetHash(certSha1Context, types.contains(Fingerprint::Type::CertSha1));
	resetHash(certSha256Context, types.contains(Fingerprint::Type::CertSha256));
}

template <class HashCtx> void updateHash(HashCtx* ctx, const uint8_t* buf, size_t len)
{
	if(ctx != nullptr) {
		ctx->update(buf, len);
	}
}

void BrClientConnection::appendCertData(const uint8_t* buf, size_t len)
{
	if(x509->count() != 0) {
		return;
	}

	updateHash(certSha1Context, buf, len);
	updateHash(certSha256Context, buf, len);
}

template <class FP, class HashCtx> bool getFp(FP& fp, HashCtx* ctx)
{
	if(ctx == nullptr) {
		return false;
	}

	fp.hash = ctx->getHash();
	return true;
}

void BrClientConnection::endCert()
{
	if(x509->count() != 0) {
		return;
	}

	delete certificate;
	certificate = new BrCertificate;
	certificate->issuer = std::move(x509->issuer);
	certificate->subject = std::move(x509->subject);
	if(certSha1Context != nullptr) {
		certificate->fpCertSha1 = new Fingerprint::Cert::Sha1{certSha1Context->getHash()};
		freeAndNil(certSha1Context);
	}
	if(certSha256Context != nullptr) {
		certificate->fpCertSha256 = new Fingerprint::Cert::Sha256{certSha256Context->getHash()};
		freeAndNil(certSha256Context);
	}
}

bool BrClientConnection::endChain()
{
	return context.session.validateCertificate();
}

} // namespace Ssl
