/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * sha2big.cpp
 *
 ****/

#include "sha2.h"

namespace
{
void sha2big_process(uint64_t state[], const uint8_t block[])
{
	using Sigma = sha2::Sigma<uint64_t, 1, 8, 7, 19, 61, 6>;
	using Sum = sha2::Sum<uint64_t, 28, 34, 39, 14, 18, 41>;

	static const uint64_t K[80] PROGMEM = {
		0x428A2F98D728AE22, 0x7137449123EF65CD, 0xB5C0FBCFEC4D3B2F, 0xE9B5DBA58189DBBC, 0x3956C25BF348B538,
		0x59F111F1B605D019, 0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118, 0xD807AA98A3030242, 0x12835B0145706FBE,
		0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2, 0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1, 0x9BDC06A725C71235,
		0xC19BF174CF692694, 0xE49B69C19EF14AD2, 0xEFBE4786384F25E3, 0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65,
		0x2DE92C6F592B0275, 0x4A7484AA6EA6E483, 0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5, 0x983E5152EE66DFAB,
		0xA831C66D2DB43210, 0xB00327C898FB213F, 0xBF597FC7BEEF0EE4, 0xC6E00BF33DA88FC2, 0xD5A79147930AA725,
		0x06CA6351E003826F, 0x142929670A0E6E70, 0x27B70A8546D22FFC, 0x2E1B21385C26C926, 0x4D2C6DFC5AC42AED,
		0x53380D139D95B3DF, 0x650A73548BAF63DE, 0x766A0ABB3C77B2A8, 0x81C2C92E47EDAEE6, 0x92722C851482353B,
		0xA2BFE8A14CF10364, 0xA81A664BBC423001, 0xC24B8B70D0F89791, 0xC76C51A30654BE30, 0xD192E819D6EF5218,
		0xD69906245565A910, 0xF40E35855771202A, 0x106AA07032BBD1B8, 0x19A4C116B8D2D0C8, 0x1E376C085141AB53,
		0x2748774CDF8EEB99, 0x34B0BCB5E19B48A8, 0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB, 0x5B9CCA4F7763E373,
		0x682E6FF3D6B2B8A3, 0x748F82EE5DEFB2FC, 0x78A5636F43172F60, 0x84C87814A1F0AB72, 0x8CC702081A6439EC,
		0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9, 0xBEF9A3F7B2C67915, 0xC67178F2E372532B, 0xCA273ECEEA26619C,
		0xD186B8C721C0C207, 0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178, 0x06F067AA72176FBA, 0x0A637DC5A2C898A6,
		0x113F9804BEF90DAE, 0x1B710B35131C471B, 0x28DB77F523047D84, 0x32CAAB7B40C72493, 0x3C9EBE0A15C9BEBC,
		0x431D67C49C100D4C, 0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A, 0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817,
	};

	sha2::process<80, Sigma, Sum>(state, block, K);
}

void sha2big_final(const crypto_sha512_context_t* ctx, uint8_t* digest, bool isSha384)
{
	decltype(ctx->state) val;
	hashFinal<true>(ctx, sha2big_process, val);
	if(isSha384) {
		Range<SHA384_SIZE / sizeof(val[0])>::encode(digest, val);
	} else {
		Range<SHA512_SIZE / sizeof(val[0])>::encode(digest, val);
	}
}
} // namespace

/*
 * SHA384
 */

CRYPTO_FUNC_INIT(sha384)
{
	static const uint64_t sha384_IV[8] PROGMEM = {
		0xCBBB9D5DC1059ED8, 0x629A292A367CD507, 0x9159015A3070DD17, 0x152FECD8F70E5939,
		0x67332667FFC00B31, 0x8EB44A8768581511, 0xDB0C2E0D64F98FA7, 0x47B5481DBEFA4FA4,
	};

	memcpy(ctx->state, sha384_IV, sizeof(sha384_IV));
	ctx->count = 0;
}

CRYPTO_FUNC_UPDATE(sha384)
{
	hashUpdate(ctx, sha2big_process, input, length);
}

CRYPTO_FUNC_FINAL(sha384)
{
	sha2big_final(ctx, digest, true);
}

CRYPTO_FUNC_GET_STATE(sha384)
{
	Range<ARRAY_SIZE(ctx->state)>::encode(static_cast<uint8_t*>(state), ctx->state);
	return ctx->count;
}

CRYPTO_FUNC_SET_STATE(sha384)
{
	Range<ARRAY_SIZE(ctx->state)>::decode(ctx->state, static_cast<const uint8_t*>(state));
	ctx->count = count;
}

/*
 * SHA512
 */

CRYPTO_FUNC_INIT(sha512)
{
	static const uint64_t sha512_IV[8] PROGMEM = {
		0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
		0x510E527FADE682D1, 0x9B05688C2B3E6C1F, 0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179,
	};

	memcpy(ctx->state, sha512_IV, sizeof(sha512_IV));
	ctx->count = 0;
}

CRYPTO_FUNC_UPDATE(sha512)
{
	crypto_sha384_update(ctx, input, length);
}

CRYPTO_FUNC_FINAL(sha512)
{
	sha2big_final(ctx, digest, false);
}

CRYPTO_FUNC_GET_STATE(sha512)
{
	return crypto_sha384_get_state(ctx, state);
}

CRYPTO_FUNC_SET_STATE(sha512)
{
	crypto_sha384_set_state(ctx, state, count);
}
