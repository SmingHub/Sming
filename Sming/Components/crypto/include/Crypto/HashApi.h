/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HashApi.h
 *
 ****/

#pragma once

#include <sming_attr.h>
#include <stddef.h>
#include <stdint.h>

#ifdef ARCH_ESP8266
#include <esp_crypto.h>
#endif

#define CRYPTO_USE_BEARSSL

#include <axtls-8266/crypto/crypto.h>
#include <bearssl_hash.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRYPTO_NAME(hash, name) crypto_##hash##_##name
#define CRYPTO_CTX(hash) CRYPTO_NAME(hash, context_t)
#define CRYPTO_FUNC_INIT(hash) void CRYPTO_NAME(hash, init)(CRYPTO_CTX(hash) * ctx)
#define CRYPTO_FUNC_UPDATE(hash)                                                                                       \
	void CRYPTO_NAME(hash, update)(CRYPTO_CTX(hash) * ctx, const void* input, uint32_t length)
#define CRYPTO_FUNC_FINAL(hash) void CRYPTO_NAME(hash, final)(uint8_t * digest, CRYPTO_CTX(hash) * ctx)
#define CRYPTO_FUNC_GET_STATE(hash) uint64_t CRYPTO_NAME(hash, get_state)(CRYPTO_CTX(hash) * ctx, void* state)
#define CRYPTO_FUNC_SET_STATE(hash)                                                                                    \
	void CRYPTO_NAME(hash, set_state)(CRYPTO_CTX(hash) * ctx, const void* state, uint64_t count)

#define CRYPTO_FUNC_HMAC(hash)                                                                                         \
	void CRYPTO_NAME(hash, hmac)(const uint8_t* msg, int msg_len, const uint8_t* key, int key_len, uint8_t* digest)
#define CRYPTO_FUNC_HMAC_V(hash)                                                                                       \
	void CRYPTO_NAME(hash, hmac_v)(const uint8_t** msg, int* msg_len, int count, const uint8_t* key, int key_len,      \
								   uint8_t* digest)

/*
 * MD5
 */

#define MD5_STATESIZE 16
#define MD5_BLOCKSIZE 64

#ifdef ARCH_ESP8266

typedef ESP_MD5_CTX CRYPTO_CTX(md5);

#define crypto_md5_init ESP_MD5_Init
#define crypto_md5_update ESP_MD5_Update
#define crypto_md5_final ESP_MD5_Final

#else

typedef MD5_CTX CRYPTO_CTX(md5);

#define crypto_md5_init MD5_Init
#define crypto_md5_update MD5_Update
#define crypto_md5_final MD5_Final

#endif

CRYPTO_FUNC_GET_STATE(md5);
CRYPTO_FUNC_SET_STATE(md5);

static inline CRYPTO_FUNC_HMAC_V(md5)
{
#ifdef ARCH_ESP8266
	ESP_hmac_md5_v(key, key_len, count, msg, msg_len, digest);
#else
	hmac_md5_v(msg, msg_len, count, key, key_len, digest);
#endif
}

static inline CRYPTO_FUNC_HMAC(md5)
{
	CRYPTO_NAME(md5, hmac_v)(&msg, &msg_len, 1, key, key_len, digest);
}

/*
 * SHA1
 */

#define SHA1_STATESIZE 20
#define SHA1_BLOCKSIZE 64

#ifdef ARCH_ESP8266
typedef ESP_SHA1_CTX CRYPTO_CTX(sha1);
#define crypto_sha1_init ESP_SHA1_Init
#define crypto_sha1_update ESP_SHA1_Update
#define crypto_sha1_final ESP_SHA1_Final
#else
typedef SHA1_CTX CRYPTO_CTX(sha1);
#define crypto_sha1_init SHA1_Init
#define crypto_sha1_update SHA1_Update
#define crypto_sha1_final SHA1_Final
#endif

CRYPTO_FUNC_GET_STATE(sha1);
CRYPTO_FUNC_SET_STATE(sha1);

static inline CRYPTO_FUNC_HMAC_V(sha1)
{
#ifdef ARCH_ESP8266
	ESP_hmac_sha1_v(key, key_len, count, msg, msg_len, digest);
#else
	hmac_sha1_v(msg, msg_len, count, key, key_len, digest);
#endif
}

static inline CRYPTO_FUNC_HMAC(sha1)
{
	crypto_sha1_hmac_v(&msg, &msg_len, 1, key, key_len, digest);
}

/*
 * SHA224
 */

#define SHA224_SIZE 28
#define SHA224_STATESIZE 32
#define SHA224_BLOCKSIZE 64

#ifdef CRYPTO_USE_BEARSSL

typedef br_sha224_context CRYPTO_CTX(sha224);

#define crypto_sha224_init br_sha224_init
#define crypto_sha224_update br_sha224_update
#define crypto_sha224_final(digest, ctx) br_sha224_out(ctx, digest)
#define crypto_sha224_get_state br_sha224_state
#define crypto_sha224_set_state br_sha224_set_state

#endif

/*
 * SHA256
 */

#define SHA256_STATESIZE 32
#define SHA256_BLOCKSIZE 64

typedef SHA256_CTX CRYPTO_CTX(sha256);

#define crypto_sha256_init SHA256_Init
#define crypto_sha256_update SHA256_Update
#define crypto_sha256_final SHA256_Final

CRYPTO_FUNC_GET_STATE(sha256);
CRYPTO_FUNC_SET_STATE(sha256);

#define crypto_sha256_hmac_v hmac_sha256_v
#define crypto_sha256_hmac hmac_sha256

/*
 * SHA384
 */

#define SHA384_STATESIZE 64
#define SHA384_BLOCKSIZE 128

#ifdef CRYPTO_USE_BEARSSL

typedef br_sha384_context CRYPTO_CTX(sha384);

#define crypto_sha384_init br_sha384_init
#define crypto_sha384_update br_sha384_update
#define crypto_sha384_final(digest, ctx) br_sha384_out(ctx, digest)
#define crypto_sha384_get_state br_sha384_state
#define crypto_sha384_set_state br_sha384_set_state

#else

typedef SHA384_CTX CRYPTO_CTX(sha384);

#define crypto_sha384_init SHA384_Init
#define crypto_sha384_update SHA384_Update
#define crypto_sha384_final SHA384_Final

CRYPTO_FUNC_GET_STATE(sha384);
CRYPTO_FUNC_SET_STATE(sha384);

#endif

/*
 * SHA512
 */

#define SHA512_STATESIZE 64
#define SHA512_BLOCKSIZE 128

#ifdef CRYPTO_USE_BEARSSL

typedef br_sha512_context CRYPTO_CTX(sha512);

#define crypto_sha512_init br_sha512_init
#define crypto_sha512_update br_sha512_update
#define crypto_sha512_final(digest, ctx) br_sha512_out(ctx, digest)
#define crypto_sha512_get_state br_sha512_state
#define crypto_sha512_set_state br_sha512_set_state

#else

typedef SHA512_CTX CRYPTO_CTX(sha512);

#define crypto_sha512_init SHA512_Init
#define crypto_sha512_update SHA512_Update
#define crypto_sha512_final SHA512_Final

CRYPTO_FUNC_GET_STATE(sha512);
CRYPTO_FUNC_SET_STATE(sha512);

#endif

#ifdef __cplusplus
}
#endif
