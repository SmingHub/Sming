/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * util.h
 *
 ****/

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stringutil.h>

namespace Crypto
{
namespace Internal
{
/*
 * Common operations
 */

template <typename T> T SHR(T x, unsigned n)
{
	return x >> n;
}

template <typename T> T ROTL(T x, unsigned n)
{
	return (x << n) | (x >> ((sizeof(T) * 8) - n));
}

template <typename T> T ROTR(T x, unsigned n)
{
	return (x << ((sizeof(T) * 8) - (n))) | (x >> (n));
}

/*
 * Byteswapping based on BearSSL code
 */

/**
 * @name Swap byte order of 32-bit value(s) and output to unaligned byte buffer
 * @{
 */
static __forceinline void encbe(uint8_t* output, uint32_t input)
{
	output[0] = uint8_t(input >> 24);
	output[1] = uint8_t(input >> 16);
	output[2] = uint8_t(input >> 8);
	output[3] = uint8_t(input);
}

static __forceinline void encbe(uint8_t* output, uint64_t input)
{
	encbe(output, uint32_t(input >> 32));
	encbe(output + 4, uint32_t(input));
}
/** @} */

/**
 * @name Decode byte array into 32-bit value(s), swapping the byte order
 * @{
 */
static __forceinline void decbe(uint32_t& output, const uint8_t* input)
{
	output = (uint32_t(input[0]) << 24) | (uint32_t(input[1]) << 16) | (uint32_t(input[2]) << 8) | input[3];
}

static __forceinline void decbe(uint64_t& output, const uint8_t* input)
{
	uint32_t x0, x1;
	decbe(x1, input);
	decbe(x0, input + 4);
	output = x0 | (uint64_t(x1) << 32);
}
/** @} */

template <size_t elementCount> class Range
{
public:
	template <typename T> static __forceinline void encode(uint8_t* output, const T* input)
	{
		encbe(output, *input);
		Range<elementCount - 1>::encode(output + sizeof(T), input + 1);
	}

	template <typename T> static __forceinline void decode(T* output, const uint8_t* input)
	{
		decbe(*output, input);
		Range<elementCount - 1>::decode(output + 1, input + sizeof(T));
	}
};

template <> class Range<1>
{
public:
	template <typename T> static __forceinline void encode(uint8_t* output, const T* input)
	{
		encbe(output, *input);
	}

	template <typename T> static __forceinline void decode(T* output, const uint8_t* input)
	{
		decbe(*output, input);
	}
};

/**
 * @name ESP algorithms store count as number of bits, so translate to bytes
 * @{
 */
template <class Context> uint64_t ESP_getCount(Context* ctx)
{
	return (ctx->count | (uint64_t(ctx->countHigh) << 32)) / 8;
}

template <class Context> void ESP_setCount(Context* ctx, uint64_t count)
{
	count *= 8;
	ctx->count = count;
	ctx->countHigh = count >> 32;
}
/** @} */

} // namespace Internal
} // namespace Crypto
