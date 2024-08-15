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
 * @retval char Corresponding character, NUL if there isn't a standard escape
 */
char escapeChar(char c)
{
	switch(c) {
	case '\0':
		return '0';
	case '\'':
		return '\'';
	case '\"':
		return '"';
	case '\?':
		return '?';
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
unsigned escapeControls(String& value)
{
	// Count number of extra characters we'll need to insert
	unsigned extra{0};
	for(auto& c : value) {
		if(escapeChar(c)) {
			extra += 1; // "\"
		} else if(c < 0x20) {
			extra += 3; // "\xnn"
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
		auto c = *in++;
		auto esc = escapeChar(c);
		if(esc) {
			*out++ = '\\';
			*out++ = esc;
		} else if(c < 0x20) {
			*out++ = '\\';
			*out++ = 'x';
			*out++ = hexchar(uint8_t(c) >> 4);
			*out++ = hexchar(uint8_t(c) & 0x0f);
		} else {
			*out++ = c;
		}
	}
	return extra;
}

} // namespace Format
