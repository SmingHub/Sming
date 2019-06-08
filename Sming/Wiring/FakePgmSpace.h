/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Support for reading flash memory
 *
 ****/

#pragma once

#include "m_printf.h"
#include "c_types.h"

// Simple check to determine if a pointer refers to flash memory
#define isFlashPtr(ptr) (uint32_t(ptr) >= 0x40200000)

#define PGM_P  const char *

#define PRIPSTR "%s"

typedef void prog_void;
typedef char prog_char;
typedef unsigned char prog_uchar;
typedef int8_t prog_int8_t;
typedef uint8_t prog_uint8_t;
typedef int16_t prog_int16_t;
typedef uint16_t prog_uint16_t;
typedef int32_t prog_int32_t;
typedef uint32_t prog_uint32_t;

/** @brief determines if the given value is aligned to a word (4-byte) boundary */
#define IS_ALIGNED(_x) (((uint32_t)(_x)&3) == 0)

// Align a size up to the nearest word boundary
#define ALIGNUP(_n) (((_n) + 3) & ~3)

// Align a size down to the nearest word boundary
#define ALIGNDOWN(_n) ((_n) & ~3)

#ifdef MFORCE32
// Your compiler supports the -mforce-l32 flag which means that
// constants can be stored in flash (program) memory instead of SRAM.
// See: https://www.arduino.cc/en/Reference/PROGMEM
#define PROGMEM_L32 PROGMEM
#else
#define PROGMEM_L32
#endif


#ifdef ICACHE_FLASH

#ifndef PROGMEM
#define PROGMEM __attribute__((aligned(4))) __attribute__((section(".irom.text")))
#endif

// flash memory must be read using 32 bit aligned addresses else a processor exception will be triggered
// order within the 32 bit values are
// --------------
// b3, b2, b1, b0
//     w1,     w0

#define pgm_read_with_offset(addr, res)                                                                                \
	__asm__("extui    %0, %1, 0, 2\n" /* Extract offset within word (in bytes) */                                      \
			"sub      %1, %1, %0\n"   /* Subtract offset from addr, yielding an aligned address */                     \
			"l32i.n   %1, %1, 0x0\n"  /* Load word from aligned address */                                             \
			"slli     %0, %0, 3\n"	/* Mulitiply offset by 8, yielding an offset in bits */                          \
			"ssr      %0\n"			  /* Prepare to shift by offset (in bits) */                                       \
			"srl      %0, %1\n"		  /* Shift right; now the requested byte is the first one */                       \
			: "=r"(res), "=r"(addr)                                                                                    \
			: "1"(addr)                                                                                                \
			:);

static inline uint8_t pgm_read_byte_inlined(const void* addr)
{
	register uint32_t res;
	pgm_read_with_offset(addr, res);
	return (uint8_t)res; /* This masks the lower byte from the returned word */
}

/* Although this says "word", it's actually 16 bit, i.e. half word on Xtensa */
static inline uint16_t pgm_read_word_inlined(const void* addr)
{
	register uint32_t res;
	pgm_read_with_offset(addr, res);
	return (uint16_t)res; /* This masks the lower half-word from the returned word */
}

// Make sure, that libraries checking existence of this macro are not failing
#define pgm_read_byte(addr) pgm_read_byte_inlined(addr)
#define pgm_read_word(addr) pgm_read_word_inlined(addr)

// No translation necessary (provided address is aligned)
#define pgm_read_dword(addr) (*(const unsigned long*)(addr))
#define pgm_read_float(addr) (*(const float*)(addr))

#ifdef __cplusplus
extern "C"
{
#endif
	void *memcpy_P(void *dest, const void *src_P, size_t length);
	int memcmp_P(const void *a1, const void *b1, size_t len);
	size_t strlen_P(const char * src_P);
	char *strcpy_P(char * dest, const char * src_P);
	char *strncpy_P(char * dest, const char * src_P, size_t size);
	int strcmp_P(const char *str1, const char *str2_P);
	int strncmp_P(const char *str1, const char *str2_P, const size_t size);
	int strcasecmp_P(const char* str1, const char* str2_P);
	char* strcat_P(char* dest, const char* src_P);
	char *strstr_P(char *haystack, const char *needle_P);

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

	#define printf_P_heap(f_P, ...)                                                                                        \
		(__extension__({                                                                                                   \
			char* __localF = (char*)malloc(strlen_P(f_P) + 1);                                                             \
			strcpy_P(__localF, f_P);                                                                                       \
			int __result = os_printf_plus(__localF, ##__VA_ARGS__);                                                        \
			free(__localF);                                                                                                \
			__result;                                                                                                      \
		}))

	#define printf_P_stack(f_P, ...)                                                                                       \
		(__extension__({                                                                                                   \
			char __localF[256];                                                                                            \
			strncpy_P(__localF, f_P, sizeof(__localF));                                                                    \
			__localF[sizeof(__localF) - 1] = '\0';                                                                         \
			m_printf(__localF, ##__VA_ARGS__);                                                                             \
		}))

	#define printf_P printf_P_stack
#ifdef __cplusplus
}
#endif

#else /* ICACHE_FLASH */

#define PROGMEM __attribute__((aligned(4)))

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#define pgm_read_float(addr) (*(const float *)(addr))

#define memcpy_P(dest, src, num) memcpy((dest), (src), (num))
#define strlen_P(a) strlen((a))
#define strcpy_P(dest, src) strcpy((dest), (src))
#define strncpy_P(dest, src, size) strncpy((dest), (src), (size))
#define strcmp_P(a, b) strcmp((a), (b))
#define strncmp_P(str1, str2_P, size) strncmp(str1, str2_P, size)
#define strcasecmp_P(a, b) strcasecmp((a), (b))
#define strcat_P(dest, src) strcat((dest), (src))
#define strstr_P(a, b) strstr((a), (b))
#define sprintf_P(s, f, ...) m_snprintf(s, 1024, f, ##__VA_ARGS__)
#define printf_P(f, ...) m_printf((f), ##__VA_ARGS__)

#endif /* ICACHE_FLASH */

/*
 * Define and use a flash string inline
 */
#define PSTR(_str)                                                                                                     \
	(__extension__({                                                                                                   \
		DEFINE_PSTR_LOCAL(__c, _str);                                                                                  \
		&__c[0];                                                                                                       \
	}))

/*
 * Declare and use a flash string inline.
 * Returns a pointer to a stack-allocated buffer of the precise size required.
 */
#define _F(_str)                                                                                                       \
	(__extension__({                                                                                                   \
		DEFINE_PSTR_LOCAL(_flash_str, _str);                                                                           \
		LOAD_PSTR(_buf, _flash_str);                                                                                   \
		_buf;                                                                                                          \
	}))


#define pgm_read_byte_near(addr) pgm_read_byte(addr)
#define pgm_read_word_near(addr) pgm_read_word(addr)
#define pgm_read_dword_near(addr) pgm_read_dword(addr)
#define pgm_read_float_near(addr) pgm_read_float(addr)
#define pgm_read_byte_far(addr) pgm_read_byte(addr)
#define pgm_read_word_far(addr) pgm_read_word(addr)
#define pgm_read_dword_far(addr) pgm_read_dword(addr)
#define pgm_read_float_far(addr) pgm_read_float(addr)

void* memcpy_aligned(void* dst, const void* src, unsigned len);
int memcmp_aligned(const void* ptr1, const void* ptr2, unsigned len);

/** @brief define a PSTR
 *  @param _name name of string
 *  @param _str the string data
 */
#define DEFINE_PSTR(_name, _str) const char _name[] PROGMEM = _str;

/** @brief define a PSTR for local (static) use
 *  @param _name name of string
 *  @param _str the string data
 */
#define DEFINE_PSTR_LOCAL(_name, _str) static DEFINE_PSTR(_name, _str)

// Declare a global reference to a PSTR instance
#define DECLARE_PSTR(_name) extern const char _name[] PROGMEM;

/*
 * Create a local (stack) buffer called _name and load it with flash data.
 * _flash_str is defined locally so the compiler knows its size (length + nul).
 * Size is rounded up to multiple of 4 bytes for fast copy.
 *
 * If defining a string within a function or other local context, must declare static.
 *
 * Example:
 * 	void testfunc() {
 * 		static DEFINE_PSTR(test, "This is a test string\n")
 * 		m_printf(LOAD_PSTR(test));
 * 	}
 *
 */
#define LOAD_PSTR(_name, _flash_str)                                                                                   \
	char _name[ALIGNUP(sizeof(_flash_str))] __attribute__((aligned(4)));                                               \
	memcpy_aligned(_name, _flash_str, sizeof(_flash_str));

#define _FLOAD(_pstr)                                                                                                  \
	(__extension__({                                                                                                   \
		LOAD_PSTR(_buf, _pstr);                                                                                        \
		_buf;                                                                                                          \
	}))

/*
 * Define a flash string and load it into a named array buffer on the stack.
 * For example, this:
 *
 * 		PSTR_ARRAY(myText, "some text");
 *
 * is roughly equivalent to this:
 *
 * 		char myText[ALIGNED_SIZE] = "some text";
 *
 * where ALIGNED_SIZE is the length of the text (including NUL terminator) rounded up to the next word boundary.
 * To get the length of the text, excluding NUL terminator, use:
 *
 * 		sizeof(PSTR_myText) - 1
 *
 */
#define PSTR_ARRAY(name, str)                                                                                        \
	static DEFINE_PSTR(PSTR_##name, str);                                                                                       \
	LOAD_PSTR(name, PSTR_##name)
