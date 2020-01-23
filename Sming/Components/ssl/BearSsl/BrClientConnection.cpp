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

#include <SslDebug.h>
#include "BrClientConnection.h"
#include <Network/Ssl/Session.h>

namespace
{
template <class HashCtx> void resetHash(std::unique_ptr<HashCtx>& ctx, bool create)
{
	ctx.reset(create ? new HashCtx : nullptr);
}

template <class HashCtx> void updateHash(HashCtx& ctx, const uint8_t* buf, size_t len)
{
	if(ctx) {
		ctx->update(buf, len);
	}
}

template <class FP, class HashCtx> void getCalculatedFingerprint(FP& fp, HashCtx& ctx)
{
	if(ctx) {
		fp.reset(new typename FP::element_type{ctx->getHash()});
		ctx.reset();
	}
}
} // namespace

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
	br_ssl_engine_set_x509(getEngine(), x509);
	if(!br_ssl_client_reset(&clientContext, context.session.hostName.c_str(), 0)) {
		debug_e("br_ssl_client_reset failed");
		return getLastError();
	}

	return startHandshake();
}

void BrClientConnection::startCert(uint32_t length)
{
	if(x509.count() != 0) {
		return;
	}

	certificate.reset(new BrCertificate);
	x509Decoder.reset(new X509Decoder(&certificate->subject, &certificate->issuer));

	auto& types = context.session.validators.fingerprintTypes;
	resetHash(certSha1Context, types.contains(Fingerprint::Type::CertSha1));
	resetHash(certSha256Context, types.contains(Fingerprint::Type::CertSha256));
}

void BrClientConnection::appendCertData(const uint8_t* buf, size_t len)
{
	if(x509.count() != 0) {
		return;
	}

	x509Decoder->push(buf, len);

	updateHash(certSha1Context, buf, len);
	updateHash(certSha256Context, buf, len);
}

void BrClientConnection::endCert()
{
	if(x509.count() != 0) {
		return;
	}

	assert(certificate);

	publicKey = x509Decoder->getPublicKey();

	getCalculatedFingerprint(certificate->fpCertSha1, certSha1Context);
	getCalculatedFingerprint(certificate->fpCertSha256, certSha256Context);

	x509Decoder.reset();
}

bool BrClientConnection::endChain()
{
	return context.session.validateCertificate();
}

} // namespace Ssl
