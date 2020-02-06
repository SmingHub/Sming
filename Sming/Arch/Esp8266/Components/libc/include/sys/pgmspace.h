/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * pgmspace.h - Support for reading flash memory (for old GCC versions)
 *
 ****/

#pragma once

#include <sys/features.h>
#include "../c_types.h"
#include "../esp_attr.h"

/**
 * @ingroup pgmspace
 * @{
 */

/**
 * @brief Place entity into flash memory
 * @brief Attach to const variable declaration to have it stored in flash memory
 *
 * Such variables should not be accessed like regular pointers as aligned instructions
 * are required. Use the provided library functions, such as `memcpy_P`, instead.
 */
#define PROGMEM STORE_ATTR ICACHE_RODATA_ATTR

/**
 * @brief Place NUL-terminated string data into flash memory
 *
 * Duplicate string data will be merged according to the rules laid out in
 * https://sourceware.org/binutils/docs/as/Section.html
 */
#define PROGMEM_PSTR                                                                                                   \
	STORE_ATTR                                                                                                         \
	__attribute__((section("\".irom0.pstr." __FILE__                                                                   \
						   "." MACROQUOTE(__LINE__) "." MACROQUOTE(__COUNTER__) "\", \"aSM\", @progbits, 1 #")))

/**
 * @brief Define and use a NUL-terminated 'C' flash string inline
 * @param str
 * @retval char[] In flash memory, access using flash functions
 * @note Uses string section merging so must not contain embedded NULs
 */
#define PSTR(str)                                                                                                      \
	(__extension__({                                                                                                   \
		static const char __pstr__[] PROGMEM_PSTR = str;                                                               \
		&__pstr__[0];                                                                                                  \
	}))

/** @} */

#ifdef __NEWLIB__

#include_next <sys/pgmspace.h>

#else

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup pgmspace
 * @{
 */

/**
 * @brief Identifies a char pointer as living in flash memory
 * Use to clarify code
 */
#define PGM_P const char*

/**
 * @brief Remove ?
 */
#define PRIPSTR "%s"

/**
 * @name AVR-compatible pgmspace types
 * Deprecated in esp8266-arduino https://github.com/esp8266/Arduino/pull/4619/
 * @{
 */
typedef void prog_void;
typedef char prog_char;
typedef unsigned char prog_uchar;
typedef int8_t prog_int8_t;
typedef uint8_t prog_uint8_t;
typedef int16_t prog_int16_t;
typedef uint16_t prog_uint16_t;
typedef int32_t prog_int32_t;
typedef uint32_t prog_uint32_t;
/** @} */

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

/**
 * @name PROGMEM-compatible C library functions
 * @{
 */

void* memcpy_P(void* dest, const void* src_P, size_t length);
int memcmp_P(const void* a1, const void* b1, size_t len);
size_t strlen_P(const char* src_P);
char* strcpy_P(char* dest, const char* src_P);
char* strncpy_P(char* dest, const char* src_P, size_t size);
int strcmp_P(const char* str1, const char* str2_P);
int strncmp_P(const char* str1, const char* str2_P, const size_t size);
int strcasecmp_P(const char* str1, const char* str2_P);
char* strcat_P(char* dest, const char* src_P);
char* strstr_P(char* haystack, const char* needle_P);

#define sprintf_P(s, f_P, ...)                                                                                         \
	(__extension__({                                                                                                   \
		int len_P = strlen_P(f_P);                                                                                     \
		int __result = 0;                                                                                              \
		char* __localF = (char*)malloc(len_P + 1);                                                                     \
		if(__localF) {                                                                                                 \
			strcpy_P(__localF, f_P);                                                                                   \
			__localF[len_P] = '\0';                                                                                    \
		}                                                                                                              \
		__result = m_snprintf(s, len_P, __localF, ##__VA_ARGS__);                                                      \
		free(__localF);                                                                                                \
		__result;                                                                                                      \
	}))

/** @} */

/**
 * @name AVR-compatibility macros
 * @{
 */
#define pgm_read_byte_near(addr) pgm_read_byte(addr)
#define pgm_read_word_near(addr) pgm_read_word(addr)
#define pgm_read_dword_near(addr) pgm_read_dword(addr)
#define pgm_read_float_near(addr) pgm_read_float(addr)
#define pgm_read_byte_far(addr) pgm_read_byte(addr)
#define pgm_read_word_far(addr) pgm_read_word(addr)
#define pgm_read_dword_far(addr) pgm_read_dword(addr)
#define pgm_read_float_far(addr) pgm_read_float(addr)
/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif // __NEWLIB__
