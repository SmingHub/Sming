#pragma once

#include <string.h>
#include <assert.h>

namespace Crypto
{
template <class Context, size_t blockSize> class Hmac
{
public:
	using Hash = typename Context::Hash;
	using Message = Blob;
	using Key = Blob;

	static Hash calculate(const Message& message, const Key& key)
	{
		return calculate(&message, 1, key);
	}

	/**
	 * @brief Calculate HMAC using fixed block size
	 * @param messages
	 * @param numMessages
	 * @param key
	 */
	static Hash calculate(const Message* messages, unsigned numMessages, const Key& key)
	{
		uint8_t k_ipad[blockSize];
		uint8_t k_opad[blockSize];
		memset(k_ipad, 0, blockSize);
		memset(k_opad, 0, blockSize);
		if(key.length <= blockSize) {
			memcpy(k_ipad, key.data, key.length);
			memcpy(k_opad, key.data, key.length);
		} else {
			auto hash = Context::calculate(key);
			memcpy(k_ipad, hash.data, hash.length);
			memcpy(k_opad, hash.data, hash.length);
		}

		for(unsigned i = 0; i < blockSize; ++i) {
			k_ipad[i] ^= 0x36;
			k_opad[i] ^= 0x5c;
		}

		Context ctx;
		ctx.update(k_ipad);
		for(unsigned i = 0; i < numMessages; ++i) {
			ctx.update(messages[i]);
		}

		auto tmp = ctx.hash();

		ctx.init();
		ctx.update(k_opad);
		ctx.update(tmp);
		return ctx.hash();
	}
};

} // namespace Crypto
