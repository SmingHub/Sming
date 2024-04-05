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

bool isFlashPtr(const void* ptr);

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

#define memcpy_P(dest, src_P, num) memcpy(dest, src_P, num)
#define memcmp_P(buf1, buf2_P, len) memcmp(buf1, buf2_P, len)
#define strlen_P(str_P) strlen(str_P)
#define strcpy_P(dest, src_P) strcpy(dest, src_P)
#define strncpy_P(dest, src_P, size) strncpy(dest, src_P, size)
#define strcmp_P(str1, str2_P) strcmp(str1, str2_P)
#define strncmp_P(str1, str2_P, size) strncmp(str1, str2_P, size)
#define strcasecmp_P(str1, str2_P) strcasecmp(str1, str2_P)
#define strcat_P(dest, src_P) strcat(dest, src_P)
#define strstr_P(haystack, needle_P) strstr(haystack, needle_P)
#define sprintf_P(str, format_P, ...) m_snprintf(str, 1024, format_P, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
