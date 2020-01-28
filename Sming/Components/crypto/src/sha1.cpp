/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "stdhash.h"
#include "../include/Crypto/HashApi/sha1.h"

namespace
{
template <typename T> T F(T b, T c, T d)
{
	return ((c ^ d) & b) ^ d;
}

template <typename T> T G(T b, T c, T d)
{
	return b ^ c ^ d;
}

template <typename T> T H(T b, T c, T d)
{
	return (d & c) | ((d | c) & b);
}

template <typename T> T I(T b, T c, T d)
{
	return G(b, c, d);
}

constexpr uint32_t K1 = 0x5A827999;
constexpr uint32_t K2 = 0x6ED9EBA1;
constexpr uint32_t K3 = 0x8F1BBCDC;
constexpr uint32_t K4 = 0xCA62C1D6;

void br_sha1_round(uint32_t state[], const uint8_t block[])
{
	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];
	uint32_t e = state[4];

	uint32_t m[80];
	Range<16>::decode(m, block);
	for(auto i = 16; i < 80; i++) {
		uint32_t x = m[i - 3] ^ m[i - 8] ^ m[i - 14] ^ m[i - 16];
		m[i] = ROTL(x, 1);
	}

	for(auto i = 0; i < 20; i += 5) {
		e += ROTL(a, 5) + F(b, c, d) + K1 + m[i + 0];
		b = ROTL(b, 30);
		d += ROTL(e, 5) + F(a, b, c) + K1 + m[i + 1];
		a = ROTL(a, 30);
		c += ROTL(d, 5) + F(e, a, b) + K1 + m[i + 2];
		e = ROTL(e, 30);
		b += ROTL(c, 5) + F(d, e, a) + K1 + m[i + 3];
		d = ROTL(d, 30);
		a += ROTL(b, 5) + F(c, d, e) + K1 + m[i + 4];
		c = ROTL(c, 30);
	}
	for(auto i = 20; i < 40; i += 5) {
		e += ROTL(a, 5) + G(b, c, d) + K2 + m[i + 0];
		b = ROTL(b, 30);
		d += ROTL(e, 5) + G(a, b, c) + K2 + m[i + 1];
		a = ROTL(a, 30);
		c += ROTL(d, 5) + G(e, a, b) + K2 + m[i + 2];
		e = ROTL(e, 30);
		b += ROTL(c, 5) + G(d, e, a) + K2 + m[i + 3];
		d = ROTL(d, 30);
		a += ROTL(b, 5) + G(c, d, e) + K2 + m[i + 4];
		c = ROTL(c, 30);
	}
	for(auto i = 40; i < 60; i += 5) {
		e += ROTL(a, 5) + H(b, c, d) + K3 + m[i + 0];
		b = ROTL(b, 30);
		d += ROTL(e, 5) + H(a, b, c) + K3 + m[i + 1];
		a = ROTL(a, 30);
		c += ROTL(d, 5) + H(e, a, b) + K3 + m[i + 2];
		e = ROTL(e, 30);
		b += ROTL(c, 5) + H(d, e, a) + K3 + m[i + 3];
		d = ROTL(d, 30);
		a += ROTL(b, 5) + H(c, d, e) + K3 + m[i + 4];
		c = ROTL(c, 30);
	}
	for(auto i = 60; i < 80; i += 5) {
		e += ROTL(a, 5) + I(b, c, d) + K4 + m[i + 0];
		b = ROTL(b, 30);
		d += ROTL(e, 5) + I(a, b, c) + K4 + m[i + 1];
		a = ROTL(a, 30);
		c += ROTL(d, 5) + I(e, a, b) + K4 + m[i + 2];
		e = ROTL(e, 30);
		b += ROTL(c, 5) + I(d, e, a) + K4 + m[i + 3];
		d = ROTL(d, 30);
		a += ROTL(b, 5) + I(c, d, e) + K4 + m[i + 4];
		c = ROTL(c, 30);
	}

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
}

} // namespace

CRYPTO_FUNC_INIT(sha1)
{
	static const uint32_t sha1_IV[5] PROGMEM = {
		0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0,
	};

	memcpy(ctx->state, sha1_IV, SHA1_STATESIZE);
	ctx->count = 0;
}

CRYPTO_FUNC_UPDATE(sha1)
{
	hashUpdate(ctx, br_sha1_round, input, length);
}

CRYPTO_FUNC_FINAL(sha1)
{
	decltype(ctx->state) val;
	hashFinal<true>(ctx, br_sha1_round, val);
	Range<SHA1_SIZE / sizeof(uint32_t)>::encode(digest, val);
}

CRYPTO_FUNC_GET_STATE(sha1)
{
	Range<SHA1_STATESIZE / 4>::encode(static_cast<uint8_t*>(state), ctx->state);
#ifdef USE_ESP_CRYPTO
	return ESP_getCount(ctx);
#else
	return ctx->count;
#endif
}

CRYPTO_FUNC_SET_STATE(sha1)
{
	Range<SHA1_STATESIZE / 4>::decode(ctx->state, static_cast<const uint8_t*>(state));
#ifdef USE_ESP_CRYPTO
	ESP_setCount(ctx, count);
#else
	ctx->count = count;
#endif
}
