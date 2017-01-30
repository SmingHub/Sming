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

#ifdef __cplusplus
}
#endif

#endif //INCLUDE_STRINGUTIL_H_
