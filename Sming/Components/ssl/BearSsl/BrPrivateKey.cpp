/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrPrivateKey.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#include <SslDebug.h>
#include "BrPrivateKey.h"
#include "CopyBuffer.h"

namespace Ssl
{
bool BrPrivateKey::decode(const uint8_t* buf, size_t len)
{
	freeMem();

	br_skey_decoder_context dc;
	br_skey_decoder_init(&dc);
	br_skey_decoder_push(&dc, buf, len);
	int err = br_skey_decoder_last_error(&dc);
	if(err != 0) {
		return false;
	}

	int type = br_skey_decoder_key_type(&dc);
	switch(type) {
	case BR_KEYTYPE_RSA:
		return copy(*br_skey_decoder_get_rsa(&dc));

	case BR_KEYTYPE_EC:
		return copy(*br_skey_decoder_get_ec(&dc));

	default:
		debug_e("Unknown key type: %d", type);
		return false;
	}
}

bool BrPrivateKey::copy(const br_rsa_private_key& rsa)
{
	freeMem();

	if(rsa.p == nullptr) {
		return true;
	}

	CopyBuffer cbuf;
	if(!cbuf.alloc(rsa.plen + rsa.qlen + rsa.dplen + rsa.dqlen + rsa.iqlen)) {
		return false;
	}

	key.type = BR_KEYTYPE_RSA;
	key.rsa = rsa;
	key.rsa.p = cbuf.copy(rsa.p, rsa.plen);
	key.rsa.q = cbuf.copy(rsa.q, rsa.qlen);
	key.rsa.dp = cbuf.copy(rsa.dp, rsa.dplen);
	key.rsa.dq = cbuf.copy(rsa.dq, rsa.dqlen);
	key.rsa.iq = cbuf.copy(rsa.iq, rsa.iqlen);

	return true;
}

bool BrPrivateKey::copy(const br_ec_private_key& ec)
{
	freeMem();

	if(ec.x == nullptr) {
		return true;
	}

	CopyBuffer cbuf;
	if(!cbuf.alloc(ec.xlen)) {
		return false;
	}

	key.type = BR_KEYTYPE_EC;
	key.ec = ec;
	key.ec.x = cbuf.copy(ec.x, ec.xlen);

	return true;
}

void BrPrivateKey::freeMem()
{
	switch(key.type) {
	case BR_KEYTYPE_RSA:
		delete[] key.rsa.p;
		break;

	case BR_KEYTYPE_EC:
		delete[] key.ec.x;
		break;
	}

	key = {};
}

} // namespace Ssl
