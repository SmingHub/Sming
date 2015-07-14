
#include "FakePgmSpace.h"

#ifdef ICACHE_FLASH
extern "C" void *memcpy_P(void *dest, const void *src_P, size_t length) {
	char *dest0 = (char *)dest;
	const char *src0 = (const char *)src_P;

	if (!((unsigned long)src_P & 3) && !(length & 3))
		return memcpy(dest, src_P, length);

	for (; length > 0; length--, src0++, dest0++)
		*dest0 = pgm_read_byte(src0);

	return dest;
}

extern "C" size_t strlen_P(const char * src_P)
{
	const char *src = (const char *)src_P;
	char val;
	int len = 0;
	for (; val; len++, src++)
		val = pgm_read_byte(src);

	return len;
}

extern "C" char *strcpy_P(char * dest, const char * src_P)
{
	int len = strlen_P(src_P);
	memcpy_P(dest, src_P, len);
	return dest;
}

extern "C" int strcmp_P(const char *str1, const char *str2_P)
{
	for (; *str1 == pgm_read_byte(str2_P); str1++, str2_P++)
		if (*str1 == '\0')
			return 0;
	return *(unsigned char *)str1 < (unsigned char)pgm_read_byte(str2_P) ? -1 : 1;
}

extern "C" char *strstr_P(char *haystack, const char *needle_P)
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

#endif
