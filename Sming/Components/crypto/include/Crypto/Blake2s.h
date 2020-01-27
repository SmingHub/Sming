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

#include "HashApi.h"
#include "Blob.h"

namespace Crypto
{
template <size_t hash_size> class Blake2sEngine
{
public:
	static_assert(hash_size > 0 && hash_size <= BLAKE2S_MAXHASHSIZE, "Blake2s invalid hashsize");

	static constexpr const char* name = "blake2s";
	static constexpr size_t hashsize = hash_size;
	static constexpr size_t blocksize = BLAKE2S_BLOCKSIZE;
	static constexpr size_t maxkeysize = BLAKE2S_MAXKEYSIZE;

	void init()
	{
		crypto_blake2s_initkey(&context, hashsize, nullptr, 0);
	}

	void init(const Secret& key)
	{
		crypto_blake2s_initkey(&context, hashsize, key.data(), key.size());
	}

	void update(const void* data, size_t size)
	{
		crypto_blake2s_update(&context, data, size);
	}

	void final(uint8_t* hash)
	{
		crypto_blake2s_final(hash, &context);
	}

private:
	crypto_blake2s_context_t context;
};

} // namespace Crypto
