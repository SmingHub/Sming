#pragma once

#include <axtls-8266/crypto/crypto.h>
#include <WString.h>
#include <Data/HexString.h>

namespace Crypto
{
/**
 * @brief
 */
struct Blob {
	const void* data;
	size_t length;

	Blob(const void* data, size_t length) : data(data), length(length)
	{
	}

	Blob(const String& str) : data(str.c_str()), length(str.length())
	{
	}
};

/**
 * @brief Class template for a Hash value, essentially just an array of bytes
 */
template <size_t size> struct HashValue {
	uint8_t data[size];

	static constexpr size_t length = size;

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
		return memcmp(data, other.data, sizeof(data)) == 0;
	}

	String toString(char separator = '\0') const
	{
		return makeHexString(data, size, separator);
	}
};

/**
 * @brief Class template for a Hash implementation 'Context'
 */
template <class Context, class Hash> class HashContext
{
public:
	/**
	 * @brief Calculate hash on a single block of data
	 * @retval Hash
	 * @{
	 */
	static Hash calculate(const void* data, size_t length)
	{
		Context ctx;
		ctx.update(data, length);
		return ctx.hash();
	}

	template <typename T> static Hash calculate(const T& blob)
	{
		Context ctx;
		ctx.update(blob);
		return ctx.hash();
	}
	/** @} */

	/**
	 * @brief Update hash over a given block of data
	 * @{
	 */
	void update(const Blob& blob)
	{
		static_cast<Context*>(this)->update(blob.data, blob.length);
	}

	void update(const String& str)
	{
		static_cast<Context*>(this)->update(str.c_str(), str.length());
	}

	/*
	 * Update from a fixed array or struct, but not a pointer - base method handles that, with size
	 */
	template <typename T> typename std::enable_if<!std::is_pointer<T>::value, void>::type update(const T& data)
	{
		static_cast<Context*>(this)->update(&data, sizeof(data));
	}
	/** @} */

	/**
	 * @brief Finalise and return the final hash value
	 * @retval Hash
	 */
	Hash hash()
	{
		Hash hash;
		static_cast<Context*>(this)->final(hash);
		return hash;
	}
};

} // namespace Crypto
