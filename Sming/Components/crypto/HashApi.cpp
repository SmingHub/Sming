#include "include/Crypto/HashApi.h"
#include "Util.h"
#include <string.h>

#define CHECK_STATESIZE(state, size) static_assert(sizeof(state) == size, "Bad state size");

/*
 * MD5
 */

CRYPTO_FUNC_GET_STATE(md5)
{
	CHECK_STATESIZE(ctx->state, MD5_STATESIZE);
	memcpy(state, ctx->state, sizeof(ctx->state));
	return (ctx->count[0] | (uint64_t(ctx->count[1]) << 32)) / 8;
}

CRYPTO_FUNC_SET_STATE(md5)
{
	memcpy(ctx->state, state, sizeof(ctx->state));
	count *= 8;
	ctx->count[0] = count;
	ctx->count[1] = count >> 32;
}

/*
 * SHA1
 */

#ifndef CRYPTO_USE_BEARSSL
CRYPTO_FUNC_GET_STATE(sha1)
{
#ifdef ARCH_ESP8266
	CHECK_STATESIZE(ctx->state, SHA1_STATESIZE);
	range_encbe(state, ctx->state, sizeof(ctx->state) / 4);
	return (ctx->count[0] | (uint64_t(ctx->count[1]) << 32)) / 8;
#else
	CHECK_STATESIZE(ctx->Intermediate_Hash, SHA1_STATESIZE);
	range_encbe(state, ctx->Intermediate_Hash, SHA1_STATESIZE / 4);
	return (ctx->Length_Low | (uint64_t(ctx->Length_High) << 32)) / 8;
#endif
}

CRYPTO_FUNC_SET_STATE(sha1)
{
#ifdef ARCH_ESP8266
	range_decbe(ctx->state, state, SHA1_STATESIZE / 4);
	count *= 8;
	ctx->count[0] = count;
	ctx->count[1] = count >> 32;
#else
	range_decbe(ctx->Intermediate_Hash, state, SHA1_STATESIZE / 4);
	ctx->Message_Block_Index = count % SHA1_BLOCKSIZE;
	count *= 8;
	ctx->Length_Low = count;
	ctx->Length_High = count >> 32;
#endif
}
#endif

/*
 * SHA256
 */

CRYPTO_FUNC_GET_STATE(sha256)
{
	CHECK_STATESIZE(ctx->state, SHA256_STATESIZE);
	range_encbe(state, ctx->state, SHA256_STATESIZE / 4);
	return ctx->total[0] | (uint64_t(ctx->total[1]) << 32);
}

CRYPTO_FUNC_SET_STATE(sha256)
{
	range_decbe(ctx->state, state, SHA256_STATESIZE / 4);
	ctx->total[0] = count;
	ctx->total[1] = count >> 32;
}

/*
 * SHA384
 */

#ifndef CRYPTO_USE_BEARSSL
CRYPTO_FUNC_GET_STATE(sha384)
{
	CHECK_STATESIZE(ctx->h_dig.h, SHA384_STATESIZE);
	range_encbe(state, ctx->h_dig.h, SHA384_STATESIZE / 8);
	return ctx->totalSize / 8;
}

CRYPTO_FUNC_SET_STATE(sha384)
{
	range_decbe(ctx->h_dig.h, state, SHA384_STATESIZE / 8);
	ctx->size = count % SHA384_BLOCKSIZE;
	ctx->totalSize = count * 8;
}
#endif

/*
 * SHA512
 */

#ifndef CRYPTO_USE_BEARSSL
CRYPTO_FUNC_GET_STATE(sha512)
{
	CHECK_STATESIZE(ctx->h_dig.h, SHA512_STATESIZE);
	range_encbe(state, ctx->h_dig.h, SHA512_STATESIZE / 8);
	return ctx->totalSize / 8;
}

CRYPTO_FUNC_SET_STATE(sha512)
{
	range_decbe(ctx->h_dig.h, state, SHA512_STATESIZE / 8);
	ctx->size = count % SHA512_BLOCKSIZE;
	ctx->totalSize = count * 8;
}
#endif
