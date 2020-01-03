/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * X509Context.cpp
 *
 ****/

#include "X509Context.h"
#include "BrConnection.h"

namespace Ssl
{
const br_x509_class X509Context::vt PROGMEM = {
	sizeof(X509Context), start_chain, start_cert, append, end_cert, end_chain, get_pkey,
};

void X509Context::startChain(const char* serverName)
{
	br_x509_decoder_init(&x509Decoder, subject.append, &subject, issuer.append, &issuer);
	certificateCount = 0;
	br_sha1_init(&certificateSha1);
	issuer.clear();
	subject.clear();
	debug_i("X509Context: serverName = \"%s\"", serverName);
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
