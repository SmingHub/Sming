/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FakePgmSpace.h - Support for reading flash memory
 *
 ****/

#pragma once

#include <esp_attr.h>
#include <sys/pgmspace.h>

#include "m_printf.h"
#include "c_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Simple check to determine if a pointer refers to flash memory
#define isFlashPtr(ptr) (uint32_t(ptr) >= 0x40200000)

/** @brief determines if the given value is aligned to a word (4-byte) boundary */
#define IS_ALIGNED(_x) (((uint32_t)(_x)&3) == 0)

// Align a size up to the nearest word boundary
#define ALIGNUP(_n) (((_n) + 3) & ~3)

// Align a size down to the nearest word boundary
#define ALIGNDOWN(_n) ((_n) & ~3)

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

/**
 * @brief Define and use a NUL-terminated 'C' flash string inline
 * @param str
 * @retval char[] In flash memory, access using flash functions
 * @note Uses string section merging so must not contain embedded NULs
 */
#undef PSTR
#define PSTR(str)                                                                                                      \
	(__extension__({                                                                                                   \
		DEFINE_PSTR_LOCAL(__pstr__, str);                                                                              \
		&__pstr__[0];                                                                                                  \
	}))

/**
 * @brief Define and use a counted flash string inline
 * @param str
 * @retval char[] In flash memory, access using flash functions
 * @note Strings are treated as binary data so may contain embedded NULs,
 * but duplicate strings are not merged.
 */
#define PSTR_COUNTED(str)                                                                                              \
	(__extension__({                                                                                                   \
		static const char __pstr__[] PROGMEM = str;                                                                    \
		&__pstr__[0];                                                                                                  \
	}))

/**
 * @brief Declare and use a flash string inline.
 * @param str
 * @retval char[] Returns a pointer to a stack-allocated buffer of the precise size required.
 */
#define _F(str)                                                                                                        \
	(__extension__({                                                                                                   \
		DEFINE_PSTR_LOCAL(__pstr__, str);                                                                              \
		LOAD_PSTR(buf, __pstr__);                                                                                      \
		buf;                                                                                                           \
	}))


/**
 * @brief copy memory aligned to word boundaries
 * @param dst
 * @param src
 * @param len Size of the source data
 *
 * dst and src must be aligned to word (4-byte) boundaries
 * `len` will be rounded up to the nearest word boundary, so the dst
 * buffer MUST be large enough for this.
 */
void* memcpy_aligned(void* dst, const void* src, unsigned len);

/**
 * @brief compare memory aligned to word boundaries
 * @param ptr1
 * @param ptr2
 * @param len
 * @retval int 0 if all bytes match
 *
 * ptr1 and ptr2 must all be aligned to word (4-byte) boundaries.
 * len is rounded up to the nearest word boundary
 */
int memcmp_aligned(const void* ptr1, const void* ptr2, unsigned len);

/** @brief define a PSTR
 *  @param name name of string
 *  @param str the string data
 */
#define DEFINE_PSTR(name, str) const char name[] PROGMEM_PSTR = str;

/** @brief define a PSTR for local (static) use
 *  @param name name of string
 *  @param str the string data
 */
#define DEFINE_PSTR_LOCAL(name, str) static DEFINE_PSTR(name, str)

/**
 * @brief Declare a global reference to a PSTR instance
 * @param name
 */
#define DECLARE_PSTR(name) extern const char name[] PROGMEM;

/**
 * @brief Create a local (stack) buffer called `name` and load it with flash data.
 * @param name
 * @param flash_str Content stored in flash. The compiler knows its size (length + nul),
 * which is rounded up to multiple of 4 bytes for fast copy.
 *
 * If defining a string within a function or other local context, must declare static.
 *
 * Example:
 *
 * 		void testfunc() {
 * 			static DEFINE_PSTR(test, "This is a test string\n");
 * 			m_printf(LOAD_PSTR(test));
 * 		}
 *
 */
#define LOAD_PSTR(name, flash_str)                                                                                   \
	char name[ALIGNUP(sizeof(flash_str))] __attribute__((aligned(4)));                                               \
	memcpy_aligned(name, flash_str, sizeof(flash_str));

#define _FLOAD(pstr)                                                                                                  \
	(__extension__({                                                                                                   \
		LOAD_PSTR(_buf, pstr);                                                                                        \
		_buf;                                                                                                          \
	}))

/**
 * @brief Define a flash string and load it into a named array buffer on the stack.
 * @note Must not contain embedded NUL characters
 *
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
#define PSTR_ARRAY(name, str)                                                                                          \
	DEFINE_PSTR_LOCAL(__pstr__##name, str);                                                                            \
	LOAD_PSTR(name, __pstr__##name)

#ifdef __cplusplus
}
#endif
