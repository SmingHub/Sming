#include "WiringFrameworkDependencies.h"
#include "FakePgmSpace.h"

#ifdef ICACHE_FLASH

void *memcpy_P(void *dest, const void *src_P, size_t length)
{
	if (IS_ALIGNED(dest) && IS_ALIGNED(src_P) && IS_ALIGNED(length))
		return memcpy_aligned(dest, src_P, length);

	auto dest0 = reinterpret_cast<char*>(dest);
	auto src0 = reinterpret_cast<const char*>(src_P);
	while (length--)
		*dest0++ = pgm_read_byte(src0++);

	return dest;
}

size_t strlen_P(const char * src_P)
{
	char val;
	size_t len = 0;
	while ((val = pgm_read_byte(src_P)) != 0)
	{
		++len;
		++src_P;
	}

	return len;
}

char *strcpy_P(char * dest, const char * src_P)
{
	for (char *p = dest; (*p = pgm_read_byte(src_P++)); p++) ;
	return dest;
}

char *strncpy_P(char * dest,const char * src_P,  size_t max_len)
{
	size_t len = strlen_P(src_P);
	if (len > max_len)
		len = max_len;
	else if (len < max_len)
		dest[len] = '\0';
	memcpy_P(dest, src_P, len);
	return dest;
}

int strcmp_P(const char *str1, const char *str2_P)
{
	for (; *str1 == pgm_read_byte(str2_P); str1++, str2_P++)
		if (*str1 == '\0')
			return 0;
	return *(unsigned char *)str1 < (unsigned char)pgm_read_byte(str2_P) ? -1 : 1;
}

char *strstr_P(char *haystack, const char *needle_P)
{
	const char *b = needle_P;
	if (pgm_read_byte(b) == 0)
		return haystack;

	for (; *haystack != 0; haystack++)
	{
		if (*haystack != pgm_read_byte(b))
			continue;

		char *a = haystack;
		while (1)
		{
			char c = pgm_read_byte(b);
			if (c == 0)
				return haystack;

			if (*a != c)
				break;

			a++;
			b++;
		}

		b = needle_P;
	}

	return 0;
}

/*
 * We implement aligned versions of some system functions to be used strictly on
 * data that is word (4-byte) aligned on all parameters.
 */

/** @brief copy memory aligned to word boundaries
 *  @param dst
 *  @param src
 *  @param len
 *  @note dst, src and len must all be aligned to word (4-byte) boundaries
 *
 */
void* memcpy_aligned(void* dst, const void* src, unsigned len)
{
	assert(IS_ALIGNED(dst) && IS_ALIGNED(src) && IS_ALIGNED(len));

	auto pd = (uint32_t*)dst;
	auto ps = (const uint32_t*)src;
	auto n = len / 4;
	while (n--)
		*pd++ = *ps++;
	return dst;
}

#endif
