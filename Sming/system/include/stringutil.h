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

  /** Return pointer to occurence of substring in string. Case insensitive.
   * \param[in] pString string to work with
   * \param[in] pToken string to locate
   * \return pointer to first occurence in of pToken in pString or NULL if not found
   */
const char* strstri(const char* pString, const char* pToken);

int strcasecmp(const char *, const char *);


static inline char hexchar(unsigned char c)
{
	return (c < 10) ? '0' + c : 'a' + c - 10;
}


#ifndef ARRAY_SIZE
# define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif


#ifdef __cplusplus
}
#endif

#endif //INCLUDE_STRINGUTIL_H_
