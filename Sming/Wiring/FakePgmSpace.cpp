/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FakePgmSpace.cpp
 *
 ****/

#include "FakePgmSpace.h"
#include <esp_systemapi.h>

/*
 * We implement aligned versions of some system functions to be used strictly on
 * data that is word (4-byte) aligned on all parameters.
 */

/** @brief copy memory aligned to word boundaries
 *  @param dst
 *  @param src
 *  @param len Size of the source data
 *  @note dst and src must be aligned to word (4-byte) boundaries
 *  `len` will be rounded up to the nearest word boundary, so the dst
 *  buffer MUST be large enough for this.
 *
 */
void* memcpy_aligned(void* dst, const void* src, unsigned len)
{
	assert(IS_ALIGNED(dst) && IS_ALIGNED(src));
	memcpy(dst, src, ALIGNUP(len));
	return dst;
}

/** @brief compare memory aligned to word boundaries
 *  @param p1
 *  @param p2
 *  @param len
 *  @retval int 0 if all bytes match
 *  @note p1 and p2 must all be aligned to word (4-byte) boundaries
 *  len is rounded up to the nearest word boundary
 */
int memcmp_aligned(const void* ptr1, const void* ptr2, unsigned len)
{
	assert(IS_ALIGNED(ptr1) && IS_ALIGNED(ptr2));

	unsigned len_aligned = ALIGNDOWN(len);
	int res = memcmp(ptr1, ptr2, len_aligned);
	if(res != 0 || len == len_aligned) {
		return res;
	}

	// Compare the remaining bytes
	auto tail1 = pgm_read_dword(reinterpret_cast<const uint8_t*>(ptr1) + len_aligned);
	auto tail2 = pgm_read_dword(reinterpret_cast<const uint8_t*>(ptr2) + len_aligned);
	return memcmp(&tail1, &tail2, len - len_aligned);
}
