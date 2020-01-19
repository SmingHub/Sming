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
#define USE_ESP_CRYPTO
#include <esp_crypto.h>
#endif

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

#define MD5_SIZE 16
#define MD5_STATESIZE 16
#define MD5_BLOCKSIZE 64

#ifdef USE_ESP_CRYPTO

#define crypto_md5_context_t ESP_MD5_CTX

#define crypto_md5_init ESP_MD5_Init
#define crypto_md5_update ESP_MD5_Update
#define crypto_md5_final ESP_MD5_Final

#else

typedef struct {
	uint32_t state[MD5_STATESIZE / 4]; ///< state (ABCD)
	uint32_t count;					   ///< Bytes in message
	uint8_t buffer[MD5_BLOCKSIZE];	 ///< Input buffer
} crypto_md5_context_t;

CRYPTO_FUNC_INIT(md5);
CRYPTO_FUNC_UPDATE(md5);
CRYPTO_FUNC_FINAL(md5);

#endif

CRYPTO_FUNC_GET_STATE(md5);
CRYPTO_FUNC_SET_STATE(md5);

#ifdef USE_ESP_CRYPTO

static inline CRYPTO_FUNC_HMAC(md5)
{
	ESP_hmac_md5_v(key, key_len, 1, &msg, &msg_len, digest);
}

static inline CRYPTO_FUNC_HMAC_V(md5)
{
	ESP_hmac_md5_v(key, key_len, count, msg, msg_len, digest);
}

#else

CRYPTO_FUNC_HMAC_V(md5);

static inline CRYPTO_FUNC_HMAC(md5)
{
	crypto_md5_hmac_v(&msg, &msg_len, 1, key, key_len, digest);
}

#endif

/*
 * SHA1
 */

#define SHA1_SIZE 20
#define SHA1_STATESIZE 20
#define SHA1_BLOCKSIZE 64

#ifdef USE_ESP_CRYPTO

#define crypto_sha1_context_t ESP_SHA1_CTX
#define crypto_sha1_init ESP_SHA1_Init
#define crypto_sha1_update ESP_SHA1_Update
#define crypto_sha1_final ESP_SHA1_Final

#else

typedef struct {
	uint32_t state[SHA1_STATESIZE / 4]; ///< Message Digest
	uint32_t count;						///< Message length in bytes
	uint8_t buffer[SHA1_BLOCKSIZE];		///< 512-bit message blocks
} crypto_sha1_context_t;

CRYPTO_FUNC_INIT(sha1);
CRYPTO_FUNC_UPDATE(sha1);
CRYPTO_FUNC_FINAL(sha1);

#endif

CRYPTO_FUNC_GET_STATE(sha1);
CRYPTO_FUNC_SET_STATE(sha1);

#ifdef USE_ESP_CRYPTO

static inline CRYPTO_FUNC_HMAC(sha1)
{
	ESP_hmac_sha1_v(key, key_len, 1, &msg, &msg_len, digest);
}

static inline CRYPTO_FUNC_HMAC_V(sha1)
{
	ESP_hmac_sha1_v(key, key_len, count, msg, msg_len, digest);
}

#else

CRYPTO_FUNC_HMAC_V(sha1);

static inline CRYPTO_FUNC_HMAC(sha1)
{
	crypto_sha1_hmac_v(&msg, &msg_len, 1, key, key_len, digest);
}

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
