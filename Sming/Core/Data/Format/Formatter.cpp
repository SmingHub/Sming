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

namespace
{
/**
 * @brief Get character used for standard escapes
 * @param c Code to be escaped
 * @param unicode Affects escaping of NUL
 * @retval char Corresponding character, NUL if there isn't a standard escape
 */
char escapeChar(char c, bool unicode)
{
	switch(c) {
	case '\0':
		return unicode ? '\0' : '0';
	case '\"':
		return '"';
	case '\\':
		return '\\';
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

} // namespace

namespace Format
{
unsigned escapeControls(String& value, bool unicode)
{
	// Count number of extra characters we'll need to insert
	unsigned extra{0};
	for(auto& c : value) {
		if(escapeChar(c, unicode)) {
			extra += 1; // "\"
		} else if(uint8_t(c) < 0x20) {
			extra += unicode ? 5 : 3; // "\uNNNN" or "\xnn"
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
		auto esc = escapeChar(c, unicode);
		if(esc) {
			*out++ = '\\';
			*out++ = esc;
		} else if(c < 0x20) {
			*out++ = '\\';
			if(unicode) {
				*out++ = 'u';
				*out++ = '0';
				*out++ = '0';
			} else {
				*out++ = 'x';
			}
			*out++ = hexchar(uint8_t(c) >> 4);
			*out++ = hexchar(uint8_t(c) & 0x0f);
		} else {
			*out++ = c;
		}
	}
	return extra;
}

} // namespace Format
