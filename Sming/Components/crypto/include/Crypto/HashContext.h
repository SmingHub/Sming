/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HashContext.h
 *
 ****/

#pragma once

#include "HashEngine.h"
#include "Blob.h"
#include "ByteArray.h"

namespace Crypto
{
/**
 * @brief Class template for a Hash implementation 'Context'
 * @tparam Engine The HashEngine implementation
 */
template <class Engine_> class HashContext
{
public:
	using Engine = Engine_;
	using Hash = ByteArray<Engine::hashsize>;

	HashContext()
	{
		reset();
	}

	/**
	 * @brief Reset the context for a new calculation
	 */
	void reset()
	{
		engine.init();
	}

	/**
	 * @name Calculate hash on some data blocks
	 * @param args See update() methods
	 * @retval Hash
	 */
	template <typename... Ts> static Hash calculate(Ts&&... args)
	{
		HashContext<Engine> ctx;
		ctx.update(std::forward<Ts>(args)...);
		return ctx.getHash();
	}

	/**
	 * @name Update hash over a given block of data
	 * @{
	 */
	void update(const Blob& blob)
	{
		engine.update(blob.data(), blob.size());
	}

	void update(const FSTR::ObjectBase& obj)
	{
		uint8_t buf[256];
		size_t offset = 0;
		size_t len;
		while((len = obj.read(offset, buf, sizeof(buf))) > 0) {
			engine.update(buf, len);
			offset += len;
		}
	}

	void update(const void* data, size_t size)
	{
		engine.update(data, size);
	}

	template <size_t size_> void update(const ByteArray<size_>& array)
	{
		engine.update(array.data(), array.size());
	}
	/** @} */

	/**
	 * @brief Finalise and return the final hash value
	 * @retval Hash
	 */
	Hash getHash()
	{
		Hash hash;
		engine.final(hash.data());
		return hash;
	}

private:
	Engine engine;
};

/*
 * Context definitions
 */

using Md5 = HashContext<Md5Engine>;
using Sha1 = HashContext<Sha1Engine>;
using Sha224 = HashContext<Sha224Engine>;
using Sha256 = HashContext<Sha256Engine>;
using Sha384 = HashContext<Sha384Engine>;
using Sha512 = HashContext<Sha512Engine>;

} // namespace Crypto
