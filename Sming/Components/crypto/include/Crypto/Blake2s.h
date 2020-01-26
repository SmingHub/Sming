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
	static constexpr size_t statesize = 0; // get_state/set_state API not supported.

	static constexpr size_t maxkeysize = 32;

	void init(const Secret& key, size_t hashSize = hashsize);
	void init(size_t hashSize = hashsize)
	{
		initCommon(hashSize, 0);
	}

	void update(const void* data, size_t size);
	void final(uint8_t* hash);

	uint64_t get_state(void* stateOut)
	{
		return 0;
	}

	void set_state(const void* stateIn, uint64_t count)
	{
	}

private:
	void initCommon(size_t hashSize, size_t keySize);

	struct State {
		uint32_t h[8];
		union {
			uint32_t t[2];
			uint64_t t64;
		};
		uint8_t buffer[blocksize];
		size_t bufferLength;
		size_t hashSize; // selected hash size
	};
	State state;
};

} // namespace Crypto