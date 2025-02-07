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

/*
 * JSON requires control characters, quotes and reverse solidus (backslash) to be escaped.
 *
 * All other codepoints from 0x20 to 0xff are left unchanged.
 * This is typically UTF8 but it could be binary or some other application-defined encoding.
 *
 * Therefore no validation is performed on the data.
 */
void Json::escape(String& value) const
{
	escapeControls(value, Option::unicode | Option::doublequote | Option::backslash);
}

void Json::quote(String& value) const
{
	escape(value);
	auto len = value.length();
	if(value.setLength(len + 2)) {
		auto s = value.begin();
		memmove(s + 1, s, len);
		s[0] = '"';
		s[len + 1] = '"';
	}
}

} // namespace Format
