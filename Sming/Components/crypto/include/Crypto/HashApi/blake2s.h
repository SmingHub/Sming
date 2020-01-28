/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * blake2s.h
 *
 ****/

#pragma once

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLAKE2S_MAXHASHSIZE 32
#define BLAKE2S_MAXKEYSIZE 32
#define BLAKE2S_STATESIZE 32
#define BLAKE2S_BLOCKSIZE 64

typedef struct {
	uint32_t state[8];
	uint64_t count;
	uint8_t buffer[BLAKE2S_BLOCKSIZE];
	size_t bufferLength;
	size_t hashSize;
} crypto_blake2s_context_t;

CRYPTO_FUNC_INIT(blake2s);
void CRYPTO_NAME(blake2s, initkey)(crypto_blake2s_context_t* ctx, size_t hashSize, const void* key, size_t keySize);
CRYPTO_FUNC_UPDATE(blake2s);
CRYPTO_FUNC_FINAL(blake2s);

#ifdef __cplusplus
}
#endif
