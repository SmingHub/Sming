/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Html.cpp
 *
 * @author mikee47 <mike@sillyhouse.net> May 2019
 *
 ****/

#include "Html.h"
#include <Data/WebHelpers/escape.h>

namespace Format
{
Html html;

void Html::escape(String& value) const
{
	auto len = html_escape_len(value.c_str(), value.length());
	if(len == value.length()) {
		// No escaping required
		return;
	}

	String html;
	html.setLength(len);
	html_escape(html.begin(), html.length(), value.c_str());
	value = html;
}

} // namespace Format
