/*
 * StringUtil.h
 *
 *  Contains utility functions for working with char strings.
 *
 *  Created on: 26.01.2017
 *  Author: (github.com/)ADiea
 */

#ifndef INCLUDE_STRINGUTIL_H_
#define INCLUDE_STRINGUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "c_types.h"

/** Return pointer to occurence of substring in string. Case insensitive.
   * \param[in] pString string to work with
   * \param[in] pToken string to locate
   * \return pointer to first occurence in of pToken in pString or NULL if not found
   */
const char* strstri(const char* pString, const char* pToken);

/** @brief A case-insensitive @code{strcmp}.
	@note non-ANSI GNU C library extension
*/
int strcasecmp(const char* s1, const char* s2);

/** @brief Returns a pointer to the first occurrence of @var{needle} (length @var{needle_len}) in @var{haystack} (length @var{haystack_len}).
	@retval void* @code{NULL} if not found.
	@note non-ANSI GNU C library extension
*/
void* memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen);

#ifdef __cplusplus
}
#endif

#endif //INCLUDE_STRINGUTIL_H_
