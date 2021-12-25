/* escape.c - encode/decode URI and HTML style escapes. */
/* PUBLIC DOMAIN - Jon Mayo - Aug 20, 2007 */
#include "escape.h"
#include <stdlib.h>
#include <c_types.h>
#include <ctype.h>

// Append str to dest with checks
static unsigned safe_append(char* dest, size_t len, const char* str)
{
	unsigned len2;
	len2 = strlen(str);
	if(len2 > len) {
		return 0; /* refuse to append */
	}
	memcpy(dest, str, len2 + 1);
	return len2;
}

/* return true if 2 characters are valid hexadecimal */
static bool ishex(const char code[2])
{
	return isxdigit(code[0]) && isxdigit(code[1]);
}

/* verify with ishex() before calling */
static uint8_t unhex(const char code[2])
{
	return (unhex(code[0]) << 4) | unhex(code[1]);
}

// These characters are escaped
static bool must_escape(char c)
{
	switch(c) {
	case '\r':
	case '\n':
	case '+':
	case '~':
	case '!':
	case '#':
	case '$':
	case '%':
	case '^':
	case '&':
	case '(':
	case ')':
	case '{':
	case '}':
	case '[':
	case ']':
	case '=':
	case ':':
	case ',':
	case ';':
	case '?':
	case '\'':
	case '"':
	case '\\':
		return true;
	default:
		return false;
	}
}

unsigned uri_escape_len(const char* s, size_t len)
{
	unsigned ret;
	for(ret = 0; len > 0; len--, s++) {
		if(must_escape(*s))
			ret += 3;
		else
			ret++;
	}
	return ret;
}

/** escapes using % - identical to JavaScript/ECMAscript version of escape()
 * if dest is NULL, return malloc()'d string. needs to be free()'d
 * if src_len is negative treat src as null terminated
 * return:
 * 	0 on error (overflow of dest, allocation failure)
 *  dest or allocated pointer on success
 *  destination string will always be null terminated
 */
char* uri_escape(char* dest, size_t dest_len, const char* src, int src_len)
{
	assert(src != nullptr);

	if(src_len < 0) {
		src_len = strlen(src);
	}
	bool ret_is_allocated = !dest;
	if(ret_is_allocated) {
		/* src_len is non-negative because of earlier condition */
		assert(src_len >= 0);
		dest_len = uri_escape_len(src, src_len) + 1;
		dest = (char*)malloc(dest_len);
		if(!dest) {
			return 0; /* allocation failure */
		}
	}
	char* ret = dest;
	/* escape these values ~!#$%^&(){}[]=:,;?'"\
	 * make sure there is room in dest for a '\0' */
	for(; src_len > 0 && dest_len > 1; src++, src_len--) {
		char c = *src;
		if(must_escape(c)) {
			/* check that there is room for "%XX\0" in dest */
			if(dest_len <= 3) {
				if(ret_is_allocated)
					free(ret);
				return 0;
			}
			dest[0] = '%';
			dest[1] = hexchar(*src >> 4);
			dest[2] = hexchar(*src & 0x0f);
			dest += 3;
			dest_len -= 3;
		} else if(c == ' ') {
			*dest++ = '+';
			dest_len--;
		} else {
			*(dest++) = *src;
			dest_len--;
		}
	}
	/* check for errors - src was not fully consumed */
	if(src_len != 0) {
		if(ret_is_allocated) {
			free(ret);
		}
		return 0;
	}
	assert(dest_len >= 1);
	*dest = '\0';

	return ret;
}

/** decode urlencoded URL into ascii characters
 * if dest is NULL, return malloc()'d string. needs to be free()'d
 * if src_len is negative treat src as null terminated
 * return:
 * 	0 on error (overflow of dest, allocation failure)
 *  dest or allocated pointer on success
 */
char* uri_unescape(char* dest, size_t dest_len, const char* src, int src_len)
{
	assert(src != nullptr);
	if(src_len < 0) {
		src_len = strlen(src);
	}
	bool ret_is_allocated = !dest;
	if(ret_is_allocated) {
		dest_len = src_len + 1; /* TODO: calculate the exact needed size? */
		dest = (char*)malloc(dest_len);
		if(!dest) {
			return 0; /* allocation failure */
		}
	}
	char* ret = dest;
	for(; dest_len > 1 && src_len > 0; dest_len--, dest++) {
		if(*src == '%' && src_len >= 3 && ishex(src + 1)) {
			*dest = (char)unhex(src + 1);
			src += 3;
			src_len -= 3;
		} else if(*src == '+') {
			*dest = ' ';
			src++;
			src_len--;
		} else {
			*dest = *(src++);
			src_len--;
		}
	}
	/* check for errors - src was not fully consumed */
	if(src_len != 0) {
		if(ret_is_allocated) {
			free(ret);
		}
		return 0;
	}
	assert(dest_len >= 1);
	*dest = 0;

	return ret;
}

/* calculates the required length for html_escape */
unsigned html_escape_len(const char* s, size_t len)
{
	unsigned ret;
	for(ret = 0; len && *s; len--, s++) {
		switch(*s) {
		case '<':
			ret += 4;
			break; /* &lt; */
		case '>':
			ret += 4;
			break; /* &gt; */
		case '&':
			ret += 5;
			break; /* &amp; */
		case '"':
			ret += 6;
			break; /* &quot; */
		case '\'':
			ret += 6;
			break; /* &apos; */
		default:
			ret++;
		}
	}
	return ret;
}

/** escapes HTML entities
 * BUG: silently drops entities if there is no room
 */
void html_escape(char* dest, size_t len, const char* s)
{
	unsigned i;

	for(i = 0; i < len && *s; s++) {
		switch(*s) {
		case '<':
			i += safe_append(dest + i, len - i, "&lt;");
			break;
		case '>':
			i += safe_append(dest + i, len - i, "&gt;");
			break;
		case '&':
			i += safe_append(dest + i, len - i, "&amp;");
			break;
		case '"':
			i += safe_append(dest + i, len - i, "&quot;");
			break;
		case '\'':
			i += safe_append(dest + i, len - i, "&apos;");
			break;
		default:
			dest[i++] = *s;
		}
	}
	dest[i] = '\0';
}

String uri_escape(const char* src, int src_len)
{
	String s;
	if(src && src_len) {
		unsigned dst_len = uri_escape_len(src, src_len);
		if(s.setLength(dst_len))
			uri_escape(s.begin(), s.length() + 1, src, src_len); // +1 for nul terminator
	}
	return s;
}

char* uri_unescape_inplace(char* str)
{
	if(str) {
		auto len = strlen(str);
		uri_unescape(str, len + 1, str, len); // +1 for nul terminator
	}
	return str;
}

String& uri_unescape_inplace(String& str)
{
	if(str) {
		char* p = str.begin();
		uri_unescape(p, str.length() + 1, p, str.length()); // +1 for nul terminator
		auto len = strlen(p);
		assert(len <= str.length());
		str.setLength(len);
	}
	return str;
}
