/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * sha1.h
 *
 ****/

#pragma once

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
