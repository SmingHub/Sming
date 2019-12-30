#include "X509Context.h"
#include "BrConnection.h"

namespace Ssl
{
const br_x509_class X509Context::vt PROGMEM = {
	sizeof(X509Context), start_chain, start_cert, append, end_cert, end_chain, get_pkey,
};

void X509Context::startChain(const char* serverName)
{
	auto subject_dn_append = [](void* ctx, const void* buf, size_t len) {
		GET_SELF();
		br_sha256_update(&self->subjectSha256, buf, len);
	};

	auto issuer_dn_append = [](void* ctx, const void* buf, size_t len) {
		GET_SELF();
		br_sha256_update(&self->issuerSha256, buf, len);
	};

	br_x509_decoder_init(&x509Decoder, subject_dn_append, this, issuer_dn_append, this);
	certificateCount = 0;
	br_sha1_init(&certificateSha1);
	br_sha256_init(&subjectSha256);
	br_sha256_init(&issuerSha256);
	(void)serverName;
}

unsigned X509Context::endChain()
{
	if(certificateCount == 0) {
		debug_w("No certificate processed");
		return BR_ERR_X509_EMPTY_CHAIN;
	}

	if(!onValidate()) {
		return BR_ERR_X509_NOT_TRUSTED;
	}

	return BR_ERR_OK;
}

} // namespace Ssl
