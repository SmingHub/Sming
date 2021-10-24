/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Standard.cpp
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#include "Standard.h"

namespace Format
{
Standard standard;

void Standard::quote(String& value) const
{
	value.replace("\"", "\"\"");
	char cQuote{'"'};
	auto len = value.length();
	value.setLength(len + 2);
	memmove(&value[1], value.c_str(), len);
	value[0] = cQuote;
	value[len + 1] = cQuote;
}

void Standard::unQuote(String& value) const
{
	char quote = value[0];
	if(quote == '"' || quote == '\'') {
		auto len = value.length();
		if(len > 1 && value[len - 1] == quote) {
			value.remove(len - 1, 1);
			value.remove(0, 1);
		}
	}
}

} // namespace Format
