/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * api.h - Macros for constructing C API for crypto functions
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
