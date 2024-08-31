/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Json.h
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#pragma once

#include "Standard.h"

namespace Format
{
class Json : public Standard
{
public:
	void escape(String& value) const override;
	void quote(String& value) const override;

	MimeType mimeType() const override
	{
		return MIME_JSON;
	}
};

extern Json json;

} // namespace Format
