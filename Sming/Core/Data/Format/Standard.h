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

	void quote(String& value) const override;
	void unQuote(String& value) const override;

	MimeType mimeType() const override
	{
		return MIME_TEXT;
	}
};

extern Standard standard;

} // namespace Format
