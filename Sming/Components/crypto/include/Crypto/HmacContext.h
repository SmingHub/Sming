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

	/**
	 * @brief Default HMAC constructor
	 *
	 * Must call init() first.
	 */
	HmacContext() = default;

	/**
	 * @brief Initialise HMAC context with key
	 */
	HmacContext(const Secret& key)
	{
		init(key);
	}

	/**
	 * @brief Initialise HMAC with key
	 * @retval Reference to enable method chaining
	 */
	HmacContext& init(const Secret& key)
	{
		ByteArray<blocksize> inputPad{};
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

		return *this;
	}

	/**
	 * @brief Update HMAC with some message content
	 * @param args See HashContext update() methods
	 * @retval Reference to enable method chaining
	 */
	template <typename... Ts> HmacContext& update(Ts&&... args)
	{
		ctx.update(std::forward<Ts>(args)...);
		return *this;
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
	 * @brief Calculate hash for some data
	 *
	 * Use like this:
	 *
	 * 		auto hash = Crypto::HmacMd5(mySecret).calculate(myData);
	 *
	 * @param args See HashContext update() methods
	 * @retval Hash
	 */
	template <typename... Ts> Hash calculate(Ts&&... args)
	{
		ctx.update(std::forward<Ts>(args)...);
		return getHash();
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

template <size_t hashsize> using HmacBlake2s = HmacContext<Blake2s<hashsize>>;
using HmacBlake2s256 = HmacBlake2s<32>;
using HmacBlake2s128 = HmacBlake2s<16>;

} // namespace Crypto
