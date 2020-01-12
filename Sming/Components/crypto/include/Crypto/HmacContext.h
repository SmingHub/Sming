/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HmacContext.h
 *
 ****/

#pragma once

#include "HashContext.h"
#include <cstring>
#include <cassert>

namespace Crypto
{
template <class HashContext> class HmacContext
{
public:
	using Engine = typename HashContext::Engine;
	using Hash = typename HashContext::Hash;
	static constexpr size_t blocksize = Engine::blocksize;

	void init(const Blob& key)
	{
		ByteArray<blocksize> inputPad;
		inputPad = {};
		if(key.size() <= blocksize) {
			memcpy(inputPad.data(), key.data(), key.size());
		} else {
			ctx.reset();
			ctx.update(key);
			auto hash = ctx.getHash();
			memcpy(inputPad.data(), hash.data(), hash.size());
		}

		outputPad = inputPad;

		for(auto& c : inputPad) {
			c ^= 0x36;
		}
		for(auto& c : outputPad) {
			c ^= 0x5c;
		}

		ctx.reset();
		ctx.update(inputPad);
	}

	void init(const void* key, size_t keySize)
	{
		init(Blob(key, keySize));
	}

	void init(const FSTR::ObjectBase& key)
	{
		uint8_t buf[key.size()];
		key.read(0, buf, sizeof(buf));
		init(buf, key.length());
	}

	/**
	 * @name Update HMAC with some message content
	 * @param args See HashContext update() methods
	 */
	template <typename... Ts> void update(Ts&&... args)
	{
		ctx.update(std::forward<Ts>(args)...);
	}

	Hash getHash()
	{
		auto tmp = ctx.getHash();

		ctx.reset();
		ctx.update(outputPad);
		ctx.update(tmp);
		return ctx.getHash();
	}

	/**
	 * @name Calculate hash on some messages
	 * @param args See HashContext update() methods
	 * @retval Hash
	 */
	template <class Key, typename... Ts> static Hash calculate(const Key& key, Ts&&... args)
	{
		HmacContext<HashContext> hmac;
		hmac.init(key);
		hmac.update(std::forward<Ts>(args)...);
		return hmac.getHash();
	}

private:
	ByteArray<blocksize> outputPad;
	HashContext ctx;
};

/*
 * HMAC definitions
 */

using HmacMd5 = HmacContext<Md5>;
using HmacSha1 = HmacContext<Sha1>;
using HmacSha224 = HmacContext<Sha224>;
using HmacSha256 = HmacContext<Sha256>;
using HmacSha384 = HmacContext<Sha384>;
using HmacSha512 = HmacContext<Sha512>;

} // namespace Crypto
