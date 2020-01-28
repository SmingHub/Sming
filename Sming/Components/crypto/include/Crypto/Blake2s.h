/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Blake2s.h
 *
 ****/

#pragma once

#include "HashApi/blake2s.h"
#include "HashEngine.h"
#include "HashContext.h"
#include "HmacContext.h"

namespace Crypto
{
#define BLAKE2S_INIT(name_)                                                                                            \
	static_assert(hash_size > 0 && hash_size <= BLAKE2S_MAXHASHSIZE, "Blake2s invalid hashsize");                      \
                                                                                                                       \
	void init()                                                                                                        \
	{                                                                                                                  \
		CRYPTO_NAME(name_, initkey)(&ctx, hash_size, nullptr, 0);                                                      \
	}                                                                                                                  \
                                                                                                                       \
	void init(const Secret& key)                                                                                       \
	{                                                                                                                  \
		CRYPTO_NAME(name_, initkey)(&ctx, hash_size, key.data(), key.size());                                          \
	}

template <size_t hash_size>
CRYPTO_HASH_ENGINE(Blake2s, blake2s, hash_size, BLAKE2S_STATESIZE, BLAKE2S_BLOCKSIZE, BLAKE2S_INIT)

/*
 * Hash Contexts
 */
template <size_t hashsize>
using Blake2s = HashContext<Blake2sEngine<hashsize>>;
using Blake2s256 = Blake2s<32>;
using Blake2s128 = Blake2s<16>;

/*
 * HMAC Contexts
 */
template <size_t hashsize> using HmacBlake2s = HmacContext<Blake2s<hashsize>>;
using HmacBlake2s256 = HmacBlake2s<32>;
using HmacBlake2s128 = HmacBlake2s<16>;

} // namespace Crypto
