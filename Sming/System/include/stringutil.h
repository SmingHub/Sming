/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StringUtil.h
 *
 *  Contains utility functions for working with char strings.
 *
 *  Created on: 26.01.2017
 *  Author: (github.com/)ADiea
 *
 ****/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"

/** @brief Return pointer to occurrence of substring in string. Case insensitive.
   * @param[in] pString string to work with
   * @param[in] pToken string to locate
   * @retval const char* pointer to first occurrence in of pToken in pString or NULL if not found
   */
const char* strstri(const char* pString, const char* pToken);

#ifndef _GNU_SOURCE
/** @brief A case-insensitive `strcmp()`.
	@note non-ANSI GNU C library extension
*/
int strcasecmp(const char* s1, const char* s2);

/** @brief Returns a pointer to the first occurrence of @var{needle} (length @var{needle_len}) in @var{haystack} (length @var{haystack_len}).
 *  @param haystack
 *  @param haystacklen
 *  @param needle
 *  @param needlelen
 *  @retval void* `NULL` if not found.
 *  @note non-ANSI GNU C library extension
*/
void* memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen);

void* memrchr(const void* s, int c, size_t n);

#endif

/**
 * @brief Compare block of memory without case sensitivity
 */
int memicmp(const void* buf1, const void* buf2, size_t len);

/**
 * @brief Return hex character corresponding to given value
 * @param c Value from 0-15, others are invalid
 * @retval char Hex character '0'-'9', 'a'-'f' or '\0' if invalid
 */
char hexchar(unsigned char c);

/**
 * @brief Return numeric value corresponding to given hex character
 * @param c Character '0'-'9', 'a'-'f' or 'A'-'F'
 * @retval int8_t Numeric value or -1 if character invalid
 */
signed char unhex(char c);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifdef __cplusplus
}
#endif
