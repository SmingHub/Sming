/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * sha2.h
 *
 ****/

#pragma once

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SHA224
 */

#define SHA224_SIZE 28
#define SHA224_STATESIZE 32
#define SHA224_BLOCKSIZE 64

typedef struct {
	uint32_t state[8];
	uint32_t count;
	uint8_t buffer[SHA224_BLOCKSIZE];
} crypto_sha224_context_t;

CRYPTO_FUNC_INIT(sha224);
CRYPTO_FUNC_UPDATE(sha224);
CRYPTO_FUNC_FINAL(sha224);
CRYPTO_FUNC_GET_STATE(sha224);
CRYPTO_FUNC_SET_STATE(sha224);

/*
 * SHA256
 */

#define SHA256_SIZE 32
#define SHA256_STATESIZE 32
#define SHA256_BLOCKSIZE 64

typedef crypto_sha224_context_t crypto_sha256_context_t;

CRYPTO_FUNC_INIT(sha256);
CRYPTO_FUNC_UPDATE(sha256);
CRYPTO_FUNC_FINAL(sha256);
CRYPTO_FUNC_GET_STATE(sha256);
CRYPTO_FUNC_SET_STATE(sha256);

CRYPTO_FUNC_HMAC_V(sha256);
static inline CRYPTO_FUNC_HMAC(sha256)
{
	crypto_sha256_hmac_v(&msg, &msg_len, 1, key, key_len, digest);
}

/*
 * SHA384
 */

#define SHA384_SIZE 48
#define SHA384_STATESIZE 64
#define SHA384_BLOCKSIZE 128

typedef struct {
	uint64_t state[8];
	uint32_t count;
	uint8_t buffer[SHA384_BLOCKSIZE];
} crypto_sha384_context_t;

CRYPTO_FUNC_INIT(sha384);
CRYPTO_FUNC_UPDATE(sha384);
CRYPTO_FUNC_FINAL(sha384);
CRYPTO_FUNC_GET_STATE(sha384);
CRYPTO_FUNC_SET_STATE(sha384);

CRYPTO_FUNC_HMAC_V(sha384);

static inline CRYPTO_FUNC_HMAC(sha384)
{
	crypto_sha384_hmac_v(&msg, &msg_len, 1, key, key_len, digest);
}

/*
 * SHA512
 */

#define SHA512_SIZE 64
#define SHA512_STATESIZE 64
#define SHA512_BLOCKSIZE 128

typedef crypto_sha384_context_t crypto_sha512_context_t;

CRYPTO_FUNC_INIT(sha512);
CRYPTO_FUNC_UPDATE(sha512);
CRYPTO_FUNC_FINAL(sha512);
CRYPTO_FUNC_GET_STATE(sha512);
CRYPTO_FUNC_SET_STATE(sha512);

CRYPTO_FUNC_HMAC_V(sha512);

static inline CRYPTO_FUNC_HMAC(sha512)
{
	crypto_sha512_hmac_v(&msg, &msg_len, 1, key, key_len, digest);
}

#ifdef __cplusplus
}
#endif
