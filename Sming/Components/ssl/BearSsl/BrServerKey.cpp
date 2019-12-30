/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrServerKey.cpp
 *
 ****/

#include "debug.h"
#include "BrServerKey.h"

namespace Ssl
{
int BrServerKey::decode(const uint8_t* buf, size_t len)
{
	freeMem();

	br_skey_decoder_context dc;
	br_skey_decoder_init(&dc);
	br_skey_decoder_push(&dc, buf, len);
	int err = br_skey_decoder_last_error(&dc);
	if(err != 0) {
		return -err;
	}

	auto copy = [](const uint8_t* buf, size_t len) {
		auto p = new uint8_t[len];
		memcpy(p, buf, len);
		return p;
	};

	keyType = Type(br_skey_decoder_key_type(&dc));
	switch(keyType) {
	case Type::RSA: {
		auto rk = br_skey_decoder_get_rsa(&dc);
		key.rsa.n_bitlen = rk->n_bitlen;
		key.rsa.p = copy(rk->p, rk->plen);
		key.rsa.plen = rk->plen;
		key.rsa.q = copy(rk->q, rk->qlen);
		key.rsa.qlen = rk->qlen;
		key.rsa.dp = copy(rk->dp, rk->dplen);
		key.rsa.dplen = rk->dplen;
		key.rsa.dq = copy(rk->dq, rk->dqlen);
		key.rsa.dqlen = rk->dqlen;
		key.rsa.iq = copy(rk->iq, rk->iqlen);
		key.rsa.iqlen = rk->iqlen;
		return BR_ERR_OK;
	}

	case Type::EC: {
		auto ek = br_skey_decoder_get_ec(&dc);
		key.ec.curve = ek->curve;
		key.ec.x = copy(ek->x, ek->xlen);
		key.ec.xlen = ek->xlen;
		return BR_ERR_OK;
	}

	default:
		debug_e("Unknown key type: %d", keyType);
		return BR_ERR_UNKNOWN_TYPE;
	}
}

void BrServerKey::freeMem()
{
	switch(keyType) {
	case Type::RSA:
		delete[] key.rsa.iq;
		delete[] key.rsa.dq;
		delete[] key.rsa.dp;
		delete[] key.rsa.q;
		delete[] key.rsa.p;
		break;

	case Type::EC:
		delete[] key.ec.x;
		break;

	case Type::None:;
	}

	keyType = Type::None;
}

} // namespace Ssl
