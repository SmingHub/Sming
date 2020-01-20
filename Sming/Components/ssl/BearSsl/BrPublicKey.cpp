/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrPublicKey.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#include <SslDebug.h>
#include "BrPublicKey.h"
#include "CopyBuffer.h"

namespace Ssl
{
bool BrPublicKey::decode(const uint8_t* buf, size_t len)
{
	freeMem();

	br_pkey_decoder_context dc;
	br_pkey_decoder_init(&dc);
	br_pkey_decoder_push(&dc, buf, len);
	int err = br_pkey_decoder_last_error(&dc);
	if(err != 0) {
		return false;
	}

	int type = br_pkey_decoder_key_type(&dc);
	switch(type) {
	case BR_KEYTYPE_RSA:
		return copy(*br_pkey_decoder_get_rsa(&dc));

	case BR_KEYTYPE_EC:
		return copy(*br_pkey_decoder_get_ec(&dc));

	default:
		debug_e("Unknown key type: %d", type);
		return false;
	}
}

bool BrPublicKey::copy(const br_rsa_public_key& rsa)
{
	freeMem();

	if(rsa.n == nullptr) {
		return true;
	}

	CopyBuffer cbuf;
	if(!cbuf.alloc(rsa.nlen + rsa.elen)) {
		return false;
	}

	key.type = BR_KEYTYPE_RSA;
	key.rsa = rsa;
	key.rsa.n = cbuf.copy(rsa.n, rsa.nlen);
	key.rsa.e = cbuf.copy(rsa.e, rsa.elen);

	return true;
}

bool BrPublicKey::copy(const br_ec_public_key& ec)
{
	freeMem();

	if(ec.q == nullptr) {
		return true;
	}

	CopyBuffer cbuf;
	if(!cbuf.alloc(ec.qlen)) {
		return false;
	}

	key.type = BR_KEYTYPE_EC;
	key.ec = ec;
	key.ec.q = cbuf.copy(ec.q, ec.qlen);

	return true;
}

void BrPublicKey::freeMem()
{
	switch(key.type) {
	case BR_KEYTYPE_RSA:
		delete[] key.rsa.n;
		break;

	case BR_KEYTYPE_EC:
		delete[] key.ec.q;
		break;
	}

	key = {};
}

} // namespace Ssl
