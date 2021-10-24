/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Xml.cpp
 *
 * @author mikee47 <mike@sillyhouse.net> Oct 2021
 *
 ****/

#include "Xml.h"

namespace Format
{
Xml xml;

void Xml::escape(String& value) const
{
	value.replace("&", "&amp;");
	value.replace("<", "&lt;");
	value.replace(">", "&gt;");
	value.replace("'", "&apos;");
	value.replace("\"", "&quot;");
}

} // namespace Format
