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

// Field names are not case sensitive
static bool headerKeyCompare(const String& a, const String& b)
{
	return a.equalsIgnoreCase(b);
}

HttpHeaders::HttpHeaders() : HashMap<String, String>(headerKeyCompare)
{
}

String HttpHeaders::toString(HttpHeaderFieldName name)
{
	if(name == HTTP_HEADER_UNKNOWN || name >= HTTP_HEADER_MAX)
		return nullptr;

	return *FieldNameStrings[name - 1];
}

HttpHeaderFieldName HttpHeaders::fromString(const String& name)
{
	// 0 is reserved for UNKNOWN
	for(unsigned i = 1; i < HTTP_HEADER_MAX; ++i) {
		if(name.equalsIgnoreCase(*FieldNameStrings[i - 1]))
			return static_cast<HttpHeaderFieldName>(i);
	}

	return HTTP_HEADER_UNKNOWN;
}
