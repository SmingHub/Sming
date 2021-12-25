/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Xml.h
 *
 * @author mikee47 <mike@sillyhouse.net> Oct 2021
 *
 ****/

#pragma once

#include "Standard.h"

namespace Format
{
class Xml : public Standard
{
public:
	void escape(String& value) const override;

	MimeType mimeType() const override
	{
		return MIME_XML;
	}
};

extern Xml xml;

} // namespace Format
