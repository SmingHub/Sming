/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Json.cpp
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#include "Json.h"
#include <debug_progmem.h>

namespace Format
{
Json json;

namespace
{
bool IsValidUtf8(const char* str, unsigned length)
{
	if(str == nullptr) {
		return true;
	}

	unsigned i = 0;
	while(i < length) {
		char c = str[i++];
		if((c & 0x80) == 0) {
			continue;
		}

		if(i >= length) {
			return false; // incomplete multibyte char
		}

		if(c & 0x20) {
			c = str[i++];
			if((c & 0xC0) != 0x80) {
				return false; // malformed trail byte or out of range char
			}
			if(i >= length) {
				return false; // incomplete multibyte char
			}
		}

		c = str[i++];
		if((c & 0xC0) != 0x80) {
			return false; // malformed trail byte
		}
	}

	return true;
}

} // namespace

/*
 * Check for invalid characters and replace them - can break browser
 * operation otherwise.
 *
 * This can occur if filenames become corrupted, so here we just
 * substitute an underscore _ for anything which fails to match UTF8.
 *
 * TODO: Perform ANSI -> UTF8 conversion?
 */
void Json::escape(String& value) const
{
	if(!IsValidUtf8(value.c_str(), value.length())) {
		debug_w("Invalid UTF8: %s", value.c_str());
		for(unsigned i = 0; i < value.length(); ++i) {
			char& c = value[i];
			if(c < 0x20 || c > 127)
				c = '_';
		}
	}

	value.replace("\"", "\\\"");
}

} // namespace Format
