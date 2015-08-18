
/*
   Copyright (c) 2014 Malte Hildingsson, malte (at) afterwi.se

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
 */

#ifndef AW_SHA1_H
#define AW_SHA1_H

#include <string.h>

#if defined(_MSC_VER)
# define _sha1_restrict __restrict
#else
# define _sha1_restrict __restrict__
#endif

#define SHA1_SIZE 20

#ifdef __cplusplus
extern "C" {
#endif

static inline void sha1mix(unsigned *_sha1_restrict r, unsigned *_sha1_restrict w) {
	unsigned a = r[0];
	unsigned b = r[1];
	unsigned c = r[2];
	unsigned d = r[3];
	unsigned e = r[4];
	unsigned t, i = 0;

#define rol(x,s) ((x) << (s) | (unsigned) (x) >> (32 - (s)))
#define mix(f,v) do { \
		t = (f) + (v) + rol(a, 5) + e + w[i & 0xf]; \
		e = d; \
		d = c; \
		c = rol(b, 30); \
		b = a; \
		a = t; \
	} while (0)

	for (; i < 16; ++i)
		mix(d ^ (b & (c ^ d)), 0x5a827999);

	for (; i < 20; ++i) {
		w[i & 0xf] = rol(w[i + 13 & 0xf] ^ w[i + 8 & 0xf] ^ w[i + 2 & 0xf] ^ w[i & 0xf], 1);
		mix(d ^ (b & (c ^ d)), 0x5a827999);
	}

	for (; i < 40; ++i) {
		w[i & 0xf] = rol(w[i + 13 & 0xf] ^ w[i + 8 & 0xf] ^ w[i + 2 & 0xf] ^ w[i & 0xf], 1);
		mix(b ^ c ^ d, 0x6ed9eba1);
	}

	for (; i < 60; ++i) {
		w[i & 0xf] = rol(w[i + 13 & 0xf] ^ w[i + 8 & 0xf] ^ w[i + 2 & 0xf] ^ w[i & 0xf], 1);
		mix((b & c) | (d & (b | c)), 0x8f1bbcdc);
	}

	for (; i < 80; ++i) {
		w[i & 0xf] = rol(w[i + 13 & 0xf] ^ w[i + 8 & 0xf] ^ w[i + 2 & 0xf] ^ w[i & 0xf], 1);
		mix(b ^ c ^ d, 0xca62c1d6);
	}

#undef mix
#undef rol

	r[0] += a;
	r[1] += b;
	r[2] += c;
	r[3] += d;
	r[4] += e;
}

static void sha1(unsigned char h[SHA1_SIZE], const void *_sha1_restrict p, size_t n) {
	size_t i = 0;
	unsigned w[16], r[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};

	for (; i < (n & ~0x3f);) {
		do w[i >> 2 & 0xf] =
			((const unsigned char *) p)[i + 3] << 0x00 |
			((const unsigned char *) p)[i + 2] << 0x08 |
			((const unsigned char *) p)[i + 1] << 0x10 |
			((const unsigned char *) p)[i + 0] << 0x18;
		while ((i += 4) & 0x3f);
		sha1mix(r, w);
	}

	memset(w, 0, sizeof w);

	for (; i < n; ++i)
		w[i >> 2 & 0xf] |= ((const unsigned char *) p)[i] << ((3 ^ i & 3) << 3);

	w[i >> 2 & 0xf] |= 0x80 << ((3 ^ i & 3) << 3);

	if ((n & 0x3f) > 56) {
		sha1mix(r, w);
		memset(w, 0, sizeof w);
	}

	w[15] = n << 3;
	sha1mix(r, w);

	for (i = 0; i < 5; ++i)
		h[(i << 2) + 0] = (unsigned char) (r[i] >> 0x18),
		h[(i << 2) + 1] = (unsigned char) (r[i] >> 0x10),
		h[(i << 2) + 2] = (unsigned char) (r[i] >> 0x08),
		h[(i << 2) + 3] = (unsigned char) (r[i] >> 0x00);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AW_SHA1_H */

