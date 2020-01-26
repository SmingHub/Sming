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
class Blake2sEngine
{
public:
	static constexpr const char* name = "blake2s";
	static constexpr size_t hashsize = 32; // Maximum hash size of Blake2s. A shorter Hash can be selected at runtime.
	static constexpr size_t blocksize = 64;
	static constexpr size_t maxkeysize = 32;

	void init(const Secret& key, size_t hashSize = hashsize);

	void init(size_t hashSize = hashsize)
	{
		initCommon(hashSize, 0);
	}

	void update(const void* data, size_t size);

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
	size_t userHashSize;
};

template <size_t hashsize_> class Blake2sEngineTemplate : public Blake2sEngine
{
public:
	static constexpr size_t hashsize = hashsize_;
	static_assert(hashsize >= 4 && (hashsize % 4) == 0, "Blake2s invalid hashsize");

	void init(const Secret& key)
	{
		Blake2sEngine::init(key, hashsize);
	}

	void init()
	{
		Blake2sEngine::init(hashsize);
	}
};

} // namespace Crypto
