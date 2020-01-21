/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * sha2small.cpp
 *
 ****/

#include "sha2.h"

namespace
{
void SHA256_Process(uint32_t state[], const uint8_t block[])
{
	using BlockShift = sha2::BlockShift<uint32_t, 7, 18, 3, 17, 19, 10>;
	using StateShift = sha2::StateShift<uint32_t, 2, 13, 22, 6, 11, 25>;

	static const uint32_t K[64] PROGMEM = {
		0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
		0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
		0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
		0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
		0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
		0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
		0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
		0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
	};

	sha2::process<64, BlockShift, StateShift>(state, block, K);
}

void crypto_sha2small_final(uint8_t* digest, crypto_sha256_context_t* ctx, bool isSha224)
{
	decltype(ctx->state) val;
	hashFinal<true>(ctx, SHA256_Process, val);
	if(isSha224) {
		Range<SHA224_SIZE / sizeof(val[0])>::encode(digest, val);
	} else {
		Range<SHA256_SIZE / sizeof(val[0])>::encode(digest, val);
	}
}

} // namespace

/*
 * SHA256
 */

CRYPTO_FUNC_INIT(sha256)
{
	static const uint32_t sha256_IV[8] PROGMEM = {
		0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19,
	};

	memcpy(ctx->state, sha256_IV, SHA256_STATESIZE);
	ctx->count = 0;
}

CRYPTO_FUNC_UPDATE(sha256)
{
	hashUpdate(ctx, SHA256_Process, input, length);
}

CRYPTO_FUNC_FINAL(sha256)
{
	crypto_sha2small_final(digest, ctx, false);
}

CRYPTO_FUNC_GET_STATE(sha256)
{
	Range<ARRAY_SIZE(ctx->state)>::encode(static_cast<uint8_t*>(state), ctx->state);
	return ctx->count;
}

CRYPTO_FUNC_SET_STATE(sha256)
{
	Range<ARRAY_SIZE(ctx->state)>::decode(ctx->state, static_cast<const uint8_t*>(state));
	ctx->count = count;
}

/*
 * SHA224
 */

CRYPTO_FUNC_INIT(sha224)
{
	static const uint32_t sha224_IV[8] PROGMEM = {
		0xC1059ED8, 0x367CD507, 0x3070DD17, 0xF70E5939, 0xFFC00B31, 0x68581511, 0x64F98FA7, 0xBEFA4FA4,
	};

	memcpy(ctx->state, sha224_IV, SHA224_STATESIZE);
	ctx->count = 0;
}

CRYPTO_FUNC_UPDATE(sha224) __attribute__((alias("crypto_sha256_update")));

CRYPTO_FUNC_FINAL(sha224)
{
	crypto_sha2small_final(digest, ctx, true);
}

CRYPTO_FUNC_GET_STATE(sha224) __attribute__((alias("crypto_sha256_get_state")));

CRYPTO_FUNC_SET_STATE(sha224) __attribute__((alias("crypto_sha256_set_state")));
