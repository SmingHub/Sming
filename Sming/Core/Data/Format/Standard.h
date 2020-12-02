/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Standard.h
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#pragma once

#include "Formatter.h"

namespace Format
{
class Standard : public Formatter
{
public:
	void escape(String& value) const override
	{
	}

	void quote(String& value) const override
	{
		char cQuote{'"'};
		auto len = value.length();
		value.setLength(len + 2);
		memmove(&value[1], value.c_str(), len);
		value[0] = cQuote;
		value[len + 1] = cQuote;
	}

	void unQuote(String& value) const override
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

	MimeType mimeType() const override
	{
		return MIME_TEXT;
	}
};

extern Standard standard;

} // namespace Format
