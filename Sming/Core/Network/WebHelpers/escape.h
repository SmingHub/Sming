#pragma once

#include <user_config.h>
#include "WString.h"

/** @brief Obtain number of characters required to escape the given text
 *  @param s source text
 *  @param len Number of characters in source text
 *  @retval unsigned number of characters for output, NOT including nul terminator
 */
unsigned uri_escape_len(const char* s, size_t len);

static inline unsigned uri_escape_len(const String& str)
{
	return uri_escape_len(str.c_str(), str.length());
}

/** @brief Escape text
 *  @param dest buffer to store result
 *  @param dest_len available space in dest; requires +1 extra character for nul terminator,
 *                  so at least uri_escape_len() + 1
 *  @param src source text to escape
 *  @param src_len number of characters in source
 *  @retval char* points to start of dest
 *  @note destination and source MUST be different buffers
 */
char* uri_escape(char* dest, size_t dest_len, const char* src, int src_len);

/** @brief unescape text
 *  @param dest buffer to store result
 *  @param dest_len available space in dest; requires +1 extra character for nul terminator
 *  @param src source text to un-escape
 *  @param src_len number of characters in source
 *  @retval char* points to start of dest
 *  @note destination and source may be the same buffer
 */
char* uri_unescape(char* dest, size_t dest_len, const char* src, int src_len);

unsigned html_escape_len(const char* s, size_t len);
void html_escape(char* dest, size_t len, const char* s);

/** @brief Replace a nul-terminated string with its unescaped version
 *  @param str the string to un-escape
 *  @retval char* the result, a copy of str
 *  @note unescaped string is never longer than escaped version
 */
char* uri_unescape_inplace(char* str);

/** @brief escape the given URI string
 *  @param src
 *  @param src_len
 *  @retval String the escaped string
 *  @note escaping may increase text size (but not always)
 */
String uri_escape(const char* src, int src_len);

static inline String uri_escape(const String& src)
{
	return src ? uri_escape(src.c_str(), src.length()) : src;
}

/** @brief replace the given text by its unescaped version
 *  @param str the string to unescape
 *  @retval reference to str, unescaped
 *  @note unescaped string is never longer than escaped version
 */
String& uri_unescape_inplace(String& str);

/** @brief return the unescaped version of a string
 *  @retval String unescaped string
 */
static inline String uri_unescape(const String& str)
{
	String ret = str;
	uri_unescape_inplace(ret);
	return ret;
}
