/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * pgmspace.h - Support for reading flash memory
 *
 ****/

#pragma once

#include <esp_attr.h>
#include <c_types.h>
#include <soc/soc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Simple check to determine if a pointer refers to flash memory
 */
#define isFlashPtr(ptr)                                                                                                \
	(((uint32_t)(ptr) >= SOC_DROM_LOW && (uint32_t)(ptr) < SOC_DROM_HIGH) ||                                           \
	 ((uint32_t)(ptr) >= SOC_IROM_LOW && (uint32_t)(ptr) < SOC_IROM_HIGH))

#define PROGMEM STORE_ATTR
#define PROGMEM_PSTR PROGMEM
#define PSTR(str) (str)

#define PGM_P const char*
#define PGM_VOID_P const void*

// flash memory must be read using 32 bit aligned addresses else a processor exception will be triggered
// order within the 32 bit values are
// --------------
// b3, b2, b1, b0
//     w1,     w0

#define pgm_read_with_offset(addr, res)                                                                                \
	__asm__("extui    %0, %1, 0, 2\n" /* Extract offset within word (in bytes) */                                      \
			"sub      %1, %1, %0\n"   /* Subtract offset from addr, yielding an aligned address */                     \
			"l32i.n   %1, %1, 0x0\n"  /* Load word from aligned address */                                             \
			"ssa8l    %0\n"			  /* Prepare to shift by offset (in bits) */                                       \
			"src      %0, %1, %1\n"   /* Shift right; now the requested byte is the first one */                       \
			: "=r"(res), "=r"(addr)                                                                                    \
			: "1"(addr)                                                                                                \
			:);

static inline uint8_t pgm_read_byte_inlined(const void* addr)
{
	uint32_t res;
	pgm_read_with_offset(addr, res);
	return (uint8_t)res; /* This masks the lower byte from the returned word */
}

/* Although this says "word", it's actually 16 bit, i.e. half word on Xtensa */
static inline uint16_t pgm_read_word_inlined(const void* addr)
{
	uint32_t res;
	pgm_read_with_offset(addr, res);
	return (uint16_t)res; /* This masks the lower half-word from the returned word */
}

/**
 * @name Macros to safely read PROGMEM locations
 * @{
 */

// Make sure, that libraries checking existence of this macro are not failing
#define pgm_read_byte(addr) pgm_read_byte_inlined(addr)
#define pgm_read_word(addr) pgm_read_word_inlined(addr)

// No translation necessary (provided address is aligned)
#define pgm_read_dword(addr) (*(const unsigned long*)(addr))
#define pgm_read_float(addr) (*(const float*)(addr))

/** @} */

#define pgm_read_byte_near(addr) pgm_read_byte(addr)
#define pgm_read_word_near(addr) pgm_read_word(addr)
#define pgm_read_dword_near(addr) pgm_read_dword(addr)
#define pgm_read_float_near(addr) pgm_read_float(addr)
#define pgm_read_byte_far(addr) pgm_read_byte(addr)
#define pgm_read_word_far(addr) pgm_read_word(addr)
#define pgm_read_dword_far(addr) pgm_read_dword(addr)
#define pgm_read_float_far(addr) pgm_read_float(addr)

#define memcpy_P(dest, src, num) memcpy(dest, src, num)
#define memcmp_P(a1, b1, len) memcmp(a1, b1, len)
#define strlen_P(a) strlen(a)
#define strcpy_P(dest, src) strcpy(dest, src)
#define strncpy_P(dest, src, size) strncpy(dest, src, size)
#define strcmp_P(a, b) strcmp(a, b)
#define strncmp_P(str1, str2_P, size) strncmp(str1, str2_P, size)
#define strcasecmp_P(a, b) strcasecmp(a, b)
#define strcat_P(dest, src) strcat(dest, src)
#define strstr_P(a, b) strstr(a, b)
#define sprintf_P(s, f, ...) m_snprintf(s, 1024, f, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
