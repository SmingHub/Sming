#pragma once

#include <axtls-8266/crypto/crypto.h>
#include <WString.h>
#include <Data/HexString.h>

namespace Crypto
{
/**
 * @brief Wraps a pointer to some data with size
 */
struct Blob {
	const void* data;
	size_t size;

	Blob(const void* data, size_t size) : data(data), size(size)
	{
	}

	Blob(const String& str) : data(str.c_str()), size(str.length())
	{
	}
};

/**
 * @brief Class template for a Hash value, essentially just an array of bytes
 */
template <class Engine_> struct HashValue {
	using Engine = Engine_;
	uint8_t data[Engine::size];

	static constexpr size_t size = sizeof(data);

	/**
	 * @brief Copy from a memory buffer
	 * @note Avoid using this method if possible as there are no checks on the data size
	 */
	bool copy(const void* src)
	{
		if(src == nullptr) {
			memset(data, 0, size);
			return false;
		}

		memcpy(data, src, size);
		return true;
	}

	bool operator==(const HashValue& other) const
	{
		return memcmp(data, other.data, size) == 0;
	}

	String toString(char separator = '\0') const
	{
		return makeHexString(data, size, separator);
	}
};

template <class Engine> String toString(const HashValue<Engine>& hash, char separator = '\0')
{
	return hash.toString(separator);
}

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
		ctx.template update(data, length);
		return ctx.hash();
	}

	template <typename T> static Hash calculate(const T& blob)
	{
		HashContext<Engine> ctx;
		ctx.template update(blob);
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

	void update(const void* data, size_t length)
	{
		engine.update(static_cast<const uint8_t*>(data), length);
	}

	/*
	 * Update from a fixed array or struct, but not a pointer - base method handles that, with size
	 */
	template <typename T>
	typename std::enable_if<std::is_standard_layout<T>::value && !std::is_pointer<T>::value, void>::type
	update(const T& data)
	{
		update(&data, sizeof(data));
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
