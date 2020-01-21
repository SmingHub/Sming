/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * sha2.h - Templated SHA2 implementation
 *
 * SHA224, SHA256, SHA384 and SHA512 all use the same algorithm, differing only in the co-efficients, constants and word sizes.
 *
 * See https://en.wikipedia.org/wiki/SHA-2 for further details.
 *
 * Standard at https://csrc.nist.gov/publications/detail/fips/180/4/final
 *
 * Some elements drawn from axTLS http://axtls.sourceforge.net/
 * and Bear SSL https://www.bearssl.org
 *
 ****/

#pragma once

#include "hash.h"

namespace Crypto
{
namespace Internal
{
namespace sha2
{
template <typename T> __forceinline T CH(T x, T y, T z)
{
	return ((y ^ z) & x) ^ z;
}

template <typename T> __forceinline T MAJ(T x, T y, T z)
{
	return (y & z) | ((y | z) & x);
}

/**
 * Provides shift operations for initial block extension.
 * Referred to in standard as lower-case sigma.
 */
template <typename T, int a0, int b0, int c0, int a1, int b1, int c1> class Sigma
{
public:
	static __forceinline T s0(T x)
	{
		return ROTR(x, a0) ^ ROTR(x, b0) ^ SHR(x, c0);
	}

	static __forceinline T s1(T x)
	{
		return ROTR(x, a1) ^ ROTR(x, b1) ^ SHR(x, c1);
	}
};

/**
 * Provides shift operations for state step
 * Referred to in standard as upper-case sigma.
 */
template <typename T, int a0, int b0, int c0, int a1, int b1, int c1> class Sum
{
public:
	static __forceinline T s0(T x)
	{
		return ROTR(x, a0) ^ ROTR(x, b0) ^ ROTR(x, c0);
	}

	static __forceinline T s1(T x)
	{
		return ROTR(x, a1) ^ ROTR(x, b1) ^ ROTR(x, c1);
	}
};

/**
 * Perform initial block extension
 */
template <class Sigma, unsigned stateLength, typename T> void extend(T w[], const uint8_t block[])
{
	Range<16>::decode(w, block);
	for(unsigned i = 16; i < stateLength; ++i) {
		w[i] = Sigma::s1(w[i - 2]) + w[i - 7] + Sigma::s0(w[i - 15]) + w[i - 16];
	}
}

/**
 * Perform a SHA2 step operation
 */
template <class Sum, typename T> __forceinline void step(T a, T b, T c, T& d, T e, T f, T g, T& h, T w, T k)
{
	T temp1 = h + Sum::s1(e) + CH(e, f, g) + k + w;
	T temp2 = Sum::s0(a) + MAJ(a, b, c);
	d += temp1;
	h = temp1 + temp2;
}

/**
 * Perform a SHA2 block step operation, consisting of a set of 8 rotated operations.
 */
template <class Sum, typename T>
__forceinline void blockStep(T& a, T& b, T& c, T& d, T& e, T& f, T& g, T& h, T w[], const T k[], unsigned i)
{
	step<Sum>(a, b, c, d, e, f, g, h, w[i + 0], k[i + 0]);
	step<Sum>(h, a, b, c, d, e, f, g, w[i + 1], k[i + 1]);
	step<Sum>(g, h, a, b, c, d, e, f, w[i + 2], k[i + 2]);
	step<Sum>(f, g, h, a, b, c, d, e, w[i + 3], k[i + 3]);
	step<Sum>(e, f, g, h, a, b, c, d, w[i + 4], k[i + 4]);
	step<Sum>(d, e, f, g, h, a, b, c, w[i + 5], k[i + 5]);
	step<Sum>(c, d, e, f, g, h, a, b, w[i + 6], k[i + 6]);
	step<Sum>(b, c, d, e, f, g, h, a, w[i + 7], k[i + 7]);
}

/**
 * Process a block of data
 */
template <unsigned len, class Sigma, class Sum, typename T>
__forceinline void process(T state[], const uint8_t block[], const T k[])
{
	T w[len];
	extend<Sigma, len>(w, block);

	T a = state[0];
	T b = state[1];
	T c = state[2];
	T d = state[3];
	T e = state[4];
	T f = state[5];
	T g = state[6];
	T h = state[7];

#ifdef CRYPTO_SHA2_STANDARD
	for(unsigned i = 0; i < len; ++i) {
		T temp1 = h + Sum::s1(e) + CH(e, f, g) + k[i] + w[i];
		T temp2 = Sum::s0(a) + MAJ(a, b, c);

		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}
#else
	for(unsigned i = 0; i < len; i += 8) {
		blockStep<Sum>(a, b, c, d, e, f, g, h, w, k, i);
	}
#endif

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	state[5] += f;
	state[6] += g;
	state[7] += h;
}

} // namespace sha2
} // namespace Internal
} // namespace Crypto
