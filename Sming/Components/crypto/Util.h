/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Util.h
 *
 ****/

/*
 * Copied from BearSSL
 *
 * @todo Will already be implemented somewhere (e.g. LWIP) find and use those instead
 */

#include <cstddef>
#include <cstdint>

static inline void encbe(uint8_t* buf, uint32_t x)
{
	buf[0] = uint8_t(x >> 24);
	buf[1] = uint8_t(x >> 16);
	buf[2] = uint8_t(x >> 8);
	buf[3] = uint8_t(x);
}

static inline void encbe(uint8_t* buf, uint64_t x)
{
	encbe(buf, uint32_t(x >> 32));
	encbe(buf + 4, uint32_t(x));
}

template <typename T> static inline void range_encbe(void* dst, const T* v, size_t num)
{
	auto buf = static_cast<uint8_t*>(dst);
	while(num-- > 0) {
		encbe(buf, *v);
		++v;
		buf += sizeof(T);
	}
}

static inline void decbe(const uint8_t* buf, uint32_t& x)
{
	x = (uint32_t(buf[0]) << 24) | (uint32_t(buf[1]) << 16) | (uint32_t(buf[2]) << 8) | buf[3];
}

static inline void decbe(const uint8_t* buf, uint64_t& x)
{
	uint32_t x0, x1;
	decbe(buf, x1);
	decbe(buf + 4, x0);
	x = x0 | (uint64_t(x1) << 32);
}

template <typename T> static inline void range_decbe(T* v, const void* src, size_t num)
{
	auto buf = static_cast<const uint8_t*>(src);
	while(num-- > 0) {
		decbe(buf, *v);
		++v;
		buf += sizeof(T);
	}
}
