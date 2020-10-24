/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpHeaderFields.cpp
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "HttpHeaderFields.h"
#include <FlashString/Vector.hpp>

// Define field name strings and a lookup table
#define XX(_tag, _str, _comment) DEFINE_FSTR_LOCAL(hhfnStr_##_tag, _str);
HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX

#define XX(_tag, _str, _comment) &hhfnStr_##_tag,
DEFINE_FSTR_VECTOR_LOCAL(fieldNameStrings, FlashString, HTTP_HEADER_FIELDNAME_MAP(XX));
#undef XX

String HttpHeaderFields::toString(HttpHeaderFieldName name) const
{
	if(name == HTTP_HEADER_UNKNOWN) {
		return nullptr;
	}

	if(name < HTTP_HEADER_CUSTOM) {
		return fieldNameStrings[name - 1];
	}

	return customFieldNames[name - HTTP_HEADER_CUSTOM];
}

String HttpHeaderFields::toString(const String& name, const String& value)
{
	String s;
	s.reserve(name.length() + 2 + value.length() + 2);
	s.concat(name);
	s.concat(": ");
	s.concat(value);
	s.concat("\r\n");
	return s;
}

HttpHeaderFieldName HttpHeaderFields::fromString(const String& name) const
{
	auto index = fieldNameStrings.indexOf(name);
	if(index >= 0) {
		return static_cast<HttpHeaderFieldName>(index + 1);
	}

	return findCustomFieldName(name);
}

HttpHeaderFieldName HttpHeaderFields::findCustomFieldName(const String& name) const
{
	auto index = customFieldNames.indexOf(name);
	if(index >= 0) {
		return static_cast<HttpHeaderFieldName>(HTTP_HEADER_CUSTOM + index);
	}

	return HTTP_HEADER_UNKNOWN;
}
