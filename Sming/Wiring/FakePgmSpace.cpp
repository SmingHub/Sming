#include "WiringFrameworkDependencies.h"
#include "FakePgmSpace.h"

#ifdef ICACHE_FLASH

void *memcpy_P(void *dest, const void *src_P, size_t length)
{
	// Yes, it seems dest must also be aligned
	if (IS_ALIGNED(dest) && IS_ALIGNED(src_P) && IS_ALIGNED(length))
		return memcpy(dest, src_P, length);

	auto dest0 = reinterpret_cast<char*>(dest);
	auto src0 = reinterpret_cast<const char*>(src_P);
	while (length--)
		*dest0++ = pgm_read_byte(src0++);

	return dest;
}

int memcmp_P(const void *a1, const void *b1, size_t len)
{
    auto a = static_cast<const uint8_t*>(a1);
    auto b = static_cast<const uint8_t*>(b1);
    for (size_t i = 0; i < len; i++, ++a, ++b) {
        uint8_t d = pgm_read_byte(a) - pgm_read_byte(b);
        if (d != 0) {
        	return d;
        }
    }
    return 0;
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


int strncmp_P(const char *str1, const char *str2_P, const size_t size)
{
	for (unsigned i=0; *str1 == pgm_read_byte(str2_P); str1++, str2_P++, i++) {
		if (i == size) {
			return 0;
		}
	}
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


int strcasecmp_P(const char* str1, const char* str2_P)
{
	for (; tolower(*str1) == tolower(pgm_read_byte(str2_P)); str1++, str2_P++)
		if (*str1 == '\0')
			return 0;
	return tolower(*(unsigned char*)str1) < tolower(pgm_read_byte(str2_P)) ? -1 : 1;
}

char* strcat_P(char* dest, const char* src_P)
{
	dest += strlen(dest);
	strcpy_P(dest, src_P);
	return dest;
}

#endif /* ICACHE_FLASH */


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
