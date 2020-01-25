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
#include "Blake2s.h"
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

	struct State {
		ByteArray<Engine::statesize> value;
		uint64_t count;
	};

	template <typename... EngineArgs> HashContext(EngineArgs&&... engineArgs)
	{
		reset(std::forward<EngineArgs>(engineArgs)...);
	}

	/**
	 * @brief Reset the context for a new calculation
	 */
	template <typename... EngineArgs> HashContext& reset(EngineArgs&&... engineArgs)
	{
		engine.init(std::forward<EngineArgs>(engineArgs)...);
		return *this;
	}

	/**
	 * @brief Calculate hash on some data
	 * @param args See update() methods
	 * @retval Hash
	 */
	template <typename... Ts> Hash calculate(Ts&&... args)
	{
		update(std::forward<Ts>(args)...);
		return getHash();
	}

	/**
	 * @name Update hash over a given block of data
	 * @{
	 */
	HashContext& update(const Blob& blob)
	{
		return update(blob.data(), blob.size());
	}

	HashContext& update(const FSTR::ObjectBase& obj)
	{
		uint8_t buf[256];
		size_t offset = 0;
		size_t len;
		while((len = obj.read(offset, buf, sizeof(buf))) > 0) {
			engine.update(buf, len);
			offset += len;
		}
		return *this;
	}

	HashContext& update(const void* data, size_t size)
	{
		engine.update(data, size);
		return *this;
	}

	template <size_t size_> HashContext& update(const ByteArray<size_>& array)
	{
		return update(array.data(), array.size());
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

	/**
	 * @brief Get intermediate hash state
	 * @param state OUT: current state
	 * @retval uint64_t Number of bytes processed so far
	 */
	State getState()
	{
		State state;
		state.count = engine.get_state(state.value.data());
		return state;
	}

	/**
	 * @brief Restore intermediate hash state
	 *
	 * Parameter values obtained via previous getState() call
	 *
	 * @param state
	 * @param count
	 */
	void setState(const State& state)
	{
		engine.set_state(state.value.data(), state.count);
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
using Blake2s = HashContext<Blake2sEngine>;

} // namespace Crypto
