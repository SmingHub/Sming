/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Formatter.cpp
 *
 * @author mikee47 <mike@sillyhouse.net> Aug 2024
 *
 ****/

#include "Formatter.h"

namespace Format
{
/**
 * @brief Get character used for standard escapes
 * @param c Code to be escaped
 * @param options
 * @retval char Corresponding character, NUL if there isn't a standard escape
 */
char escapeChar(char c, Options options)
{
	switch(c) {
	case '\0':
		return options[Option::unicode] ? '\0' : '0';
	case '\"':
		return options[Option::doublequote] ? c : '\0';
	case '\'':
		return options[Option::singlequote] ? c : '\0';
	case '\\':
		return options[Option::backslash] ? c : '\0';
	case '\a':
		return 'a';
	case '\b':
		return 'b';
	case '\f':
		return 'f';
	case '\n':
		return 'n';
	case '\r':
		return 'r';
	case '\t':
		return 't';
	case '\v':
		return 'v';
	default:
		return '\0';
	}
}

unsigned escapeControls(String& value, Options options)
{
	// Count number of extra characters we'll need to insert
	unsigned extra{0};
	for(auto& c : value) {
		if(escapeChar(c, options)) {
			extra += 1; // "\"
		} else if(options[Option::unicode]) {
			if(uint8_t(c) < 0x20) {
				extra += 5; // "\uNNNN"
			}
		} else if(uint8_t(c) < 0x20) {
			extra += 3; // "\xnn"
		} else if((c & 0x80) && options[Option::utf8]) {
			// Characters from U+0080 to U+07FF are encoded in two bytes in UTF-8
			extra += 1;
		}
	}
	if(extra == 0) {
		return 0;
	}
	auto len = value.length();
	if(!value.setLength(len + extra)) {
		return 0;
	}
	char* out = value.begin();
	const char* in = out;
	memmove(out + extra, in, len);
	in += extra;
	while(len--) {
		uint8_t c = *in++;
		auto esc = escapeChar(c, options);
		if(esc) {
			*out++ = '\\';
			c = esc;
		} else if(options[Option::unicode]) {
			if(uint8_t(c) < 0x20) {
				*out++ = '\\';
				*out++ = 'u';
				*out++ = '0';
				*out++ = '0';
				*out++ = hexchar(uint8_t(c) >> 4);
				c = hexchar(uint8_t(c) & 0x0f);
			}
		} else if(uint8_t(c) < 0x20) {
			*out++ = '\\';
			*out++ = 'x';
			*out++ = hexchar(uint8_t(c) >> 4);
			c = hexchar(uint8_t(c) & 0x0f);
		} else if((c & 0x80) && options[Option::utf8]) {
			*out++ = 0xc0 | (c >> 6);
			c = 0x80 | (c & 0x3f);
		}
		*out++ = c;
	}
	return extra;
}

} // namespace Format
