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

/** Return pointer to occurence of substring in string. Case insensitive.
   * \param[in] pString string to work with
   * \param[in] pToken string to locate
   * \return pointer to first occurence in of pToken in pString or NULL if not found
   */
const char* strstri(const char* pString, const char* pToken);

#ifndef _GNU_SOURCE
/** @brief A case-insensitive @code{strcmp}.
	@note non-ANSI GNU C library extension
*/
int strcasecmp(const char* s1, const char* s2);

/** @brief Returns a pointer to the first occurrence of @var{needle} (length @var{needle_len}) in @var{haystack} (length @var{haystack_len}).
	@retval void* @code{NULL} if not found.
	@note non-ANSI GNU C library extension
*/
void* memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen);
#endif

static inline signed char hexchar(unsigned char c)
{
	if(c < 10)
		return '0' + c;
	else if(c <= 15)
		return 'a' + c - 10;
	else
		return '\0';
}

static inline signed char unhex(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	else if(c >= 'a' && c <= 'f')
		return 10 + c - 'a';
	else if(c >= 'A' && c <= 'F')
		return 10 + c - 'A';
	else
		return -1;
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifdef __cplusplus
}
#endif
