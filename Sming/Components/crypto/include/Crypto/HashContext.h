#pragma once

#include <axtls-8266/crypto/crypto.h>
#include "Blob.h"
#include "ByteArray.h"

namespace Crypto
{
/**
 * @brief Class template for a Hash implementation 'Context'
 * @tparam Engine The hash implementation with `init`, `update` and `final` methods
 */
template <class Engine_> class HashContext
{
public:
	using Engine = Engine_;
	using Hash = typename Engine::Hash;

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
	 * @name Calculate hash on a single block of data
	 * @retval Hash
	 * @{
	 */
	static Hash calculate(const void* data, size_t length)
	{
		HashContext<Engine> ctx;
		ctx.update(data, length);
		return ctx.hash();
	}

	template <typename T> static Hash calculate(const T& blob)
	{
		HashContext<Engine> ctx;
		ctx.update(blob);
		return ctx.hash();
	}
	/** @} */

	/**
	 * @name Update hash over a given block of data
	 * @{
	 */
	void update(const Blob& blob)
	{
		update(blob.data, blob.size);
	}

	void update(const String& s)
	{
		update(s.c_str(), s.length());
	}

	void update(const FSTR::ObjectBase& obj)
	{
		LOAD_FSTR(arr, obj);
		update(arr, obj.length());
	}

	void update(const void* data, size_t length)
	{
		engine.update(static_cast<const uint8_t*>(data), length);
	}
	/** @} */

	/**
	 * @brief Finalise and return the final hash value
	 * @retval Hash
	 */
	Hash hash()
	{
		Hash hash;
		engine.final(hash);
		return hash;
	}

private:
	Engine engine;
};

} // namespace Crypto
