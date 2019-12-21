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

#include "../esp_attr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PROGMEM STORE_ATTR
#define PROGMEM_PSTR PROGMEM
#define PSTR(str) (str)

#define PGM_P const char*
#define PGM_VOID_P const void*

#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#define pgm_read_word(addr) (*(const unsigned short*)(addr))
#define pgm_read_dword(addr) (*(const unsigned long*)(addr))
#define pgm_read_float(addr) (*(const float*)(addr))

#define pgm_read_byte_near(addr) pgm_read_byte(addr)
#define pgm_read_word_near(addr) pgm_read_word(addr)
#define pgm_read_dword_near(addr) pgm_read_dword(addr)
#define pgm_read_float_near(addr) pgm_read_float(addr)
#define pgm_read_byte_far(addr) pgm_read_byte(addr)
#define pgm_read_word_far(addr) pgm_read_word(addr)
#define pgm_read_dword_far(addr) pgm_read_dword(addr)
#define pgm_read_float_far(addr) pgm_read_float(addr)

#define memcpy_P(dest, src, num) memcpy((dest), (src), (num))
#define memcmp_P(a1, b1, len) memcmp(a1, b1, len)
#define strlen_P(a) strlen((a))
#define strcpy_P(dest, src) strcpy((dest), (src))
#define strncpy_P(dest, src, size) strncpy((dest), (src), (size))
#define strcmp_P(a, b) strcmp((a), (b))
#define strncmp_P(str1, str2_P, size) strncmp(str1, str2_P, size)
#define strcasecmp_P(a, b) strcasecmp((a), (b))
#define strcat_P(dest, src) strcat((dest), (src))
#define strstr_P(a, b) strstr((a), (b))
#define sprintf_P(s, f, ...) m_snprintf(s, 1024, f, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
