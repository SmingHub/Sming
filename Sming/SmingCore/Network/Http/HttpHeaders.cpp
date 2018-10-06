/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "HttpHeaders.h"

// Define field name strings and a lookup table
#define XX(_tag, _str, _comment) static DEFINE_FSTR(hhfnStr_##_tag, _str);
HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX

static FSTR_TABLE(FieldNameStrings) = {
#define XX(_tag, _str, _comment) FSTR_PTR(hhfnStr_##_tag),
	HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX
};

String HttpHeaders::toString(HttpHeaderFieldName name) const
{
	if(name == HTTP_HEADER_UNKNOWN)
		return nullptr;

	if(name < HTTP_HEADER_CUSTOM)
		return *FieldNameStrings[name - 1];

	return _customFieldNames.getText(name - HTTP_HEADER_CUSTOM);
}

HttpHeaderFieldName HttpHeaders::fromString(const String& name) const
{
	// 0 is reserved for UNKNOWN
	for(unsigned i = 1; i < HTTP_HEADER_CUSTOM; ++i) {
		if(name.equalsIgnoreCase(*FieldNameStrings[i - 1]))
			return static_cast<HttpHeaderFieldName>(i);
	}

	auto index = _customFieldNames.indexOf(name);
	if(index >= 0)
		return static_cast<HttpHeaderFieldName>(HTTP_HEADER_CUSTOM + index);

	return HTTP_HEADER_UNKNOWN;
}
