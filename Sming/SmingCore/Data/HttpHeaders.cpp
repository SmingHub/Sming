/*
 * HttpHeaders.cpp
 *
 *  Created on: 28 Jul 2018
 *      Author: mikee47
 */


#include "HttpHeaders.h"
#include "stringutil.h"


// Define strings
#define XX(_tag, _str, _comment) \
	static const char __hhfn_str_ ## _tag [] PROGMEM = _str;
HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX


// Define lookup table
static const char* const __hhfn_table[] PROGMEM = {
	#define XX(_tag, _str, _comment) \
		__hhfn_str_ ## _tag,
		HTTP_HEADER_FIELDNAME_MAP(XX)
	#undef XX
};


String HttpHeaders::toString(HttpHeaderFieldName name)
{
	if (name == hhfn_UNKNOWN || name >= hhfn_MAX)
		return nullptr;

	char buffer[64];
	strncpy_P(buffer, __hhfn_table[name - 1], sizeof(buffer));
	return buffer;
}

HttpHeaderFieldName HttpHeaders::fromString(const String& name)
{
	for (unsigned i = 1; i < hhfn_MAX; ++i) {
		String s = toString((HttpHeaderFieldName)i);
		if (s.equalsIgnoreCase(name))
			return (HttpHeaderFieldName)i;
	}

	return hhfn_UNKNOWN;
}



