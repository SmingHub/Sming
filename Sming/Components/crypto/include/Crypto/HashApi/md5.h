/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * md5.h
 *
 ****/

#pragma once

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
