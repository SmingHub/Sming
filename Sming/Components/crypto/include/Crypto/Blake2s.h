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

#include <sming_attr.h>
#include <stdint.h>
#include <cstring>
#include <cassert>
#include "Blob.h"

namespace Crypto
{
class Blake2sImpl
{
public:
	static constexpr const char* name = "blake2s";
	static constexpr size_t maxhashsize = 32;
	static constexpr size_t blocksize = 64;
	static constexpr size_t maxkeysize = 32;

	void update(const void* data, size_t size);

protected:
	void init(const Secret& key, size_t hashSize);

	void init(size_t hashSize)
	{
		initCommon(hashSize, 0);
	}

	void final(uint8_t* hash);

private:
	void initCommon(size_t hashSize, size_t keySize);

	struct Context {
		uint32_t state[8];
		uint64_t count;
		uint8_t buffer[blocksize];
		size_t bufferLength;
	};
	Context context;
	size_t hashSize;
};

template <size_t hash_size> class Blake2sEngine : public Blake2sImpl
{
public:
	static_assert(hash_size > 0 && hash_size <= maxhashsize, "Blake2s invalid hashsize");

	static constexpr size_t hashsize = hash_size;

	void init(const Secret& key)
	{
		Blake2sImpl::init(key, hashsize);
	}

	void init()
	{
		Blake2sImpl::init(hashsize);
	}

	void final(uint8_t* hash)
	{
		Blake2sImpl::final(hash);
	}
};

} // namespace Crypto
