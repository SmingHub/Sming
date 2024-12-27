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
 * Check for invalid characters and replace them - can break browser
 * operation otherwise.
 *
 * This can occur if filenames become corrupted, so here we just
 * substitute an underscore _ for anything which fails to match UTF8.
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
