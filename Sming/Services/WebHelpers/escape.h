/*
 * 12/8/2018 (mikee47)
 *
 * 	Additional functions added to work with String objects for ease of use.
 */

#ifndef ESCAPE_H
#define ESCAPE_H

#include <user_config.h>
#include "WString.h"

unsigned uri_escape_len(const char *s, size_t len);
char *uri_escape(char *dest, size_t dest_len, const char *src, int src_len);
char *uri_unescape(char *dest, size_t dest_len, const char *src, int src_len);

unsigned html_escape_len(const char *s, size_t len);
void html_escape(char *dest, size_t len, const char *s);

/** @brief escape the given URI string
 *  @param src
 *  @param src_len
 *  @retval String the escaped string
 *  @note escaping may increase text size (but not always)
 */
String uri_escape(const char *src, int src_len);

static inline String uri_escape(const String& src)
{
	return src ? uri_escape(src.c_str(), src.length()) : src;
}


/** @brief replace the given uri by its unescaped version
 *  @retval int length of result
 *  @note unescaped string is never longer than escaped version
 */
int uri_unescape_inplace(String& s);

/** @brief return the unescaped version of a string
 *  @retval String unescaped string
 */
static inline String uri_unescape(const String& s)
{
	String ret = s;
	uri_unescape_inplace(ret);
	return ret;
}

#endif
