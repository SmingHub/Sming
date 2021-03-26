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
#define XX(tag, str, flags, comment) DEFINE_FSTR_LOCAL(hhfnStr_##tag, str);
HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX

#define XX(tag, str, flags, comment) &hhfnStr_##tag,
DEFINE_FSTR_VECTOR_LOCAL(fieldNameStrings, FlashString, HTTP_HEADER_FIELDNAME_MAP(XX));
#undef XX

HttpHeaderFields::Flags HttpHeaderFields::getFlags(HttpHeaderFieldName name) const
{
	switch(name) {
#define XX(tag, str, flags, comment)                                                                                   \
	case HttpHeaderFieldName::tag:                                                                                     \
		return flags;
		HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX
	default:
		// Custom fields
		return 0;
	}
}

String HttpHeaderFields::toString(HttpHeaderFieldName name) const
{
	if(name == HTTP_HEADER_UNKNOWN) {
		return nullptr;
	}

	if(name < HTTP_HEADER_CUSTOM) {
		return fieldNameStrings[unsigned(name) - 1];
	}

	return customFieldNames[unsigned(name) - unsigned(HTTP_HEADER_CUSTOM)];
}

String HttpHeaderFields::toString(HttpHeaderFieldName name, const String& value) const
{
	if(getFlags(name)[Flag::Multi]) {
		String s;
		Vector<String> splits;
		String values(value);
		int m = splitString(values, '\0', splits);
		for(int i = 0; i < m; i++) {
			s += toString(toString(name), splits[i]);
		}

		return s;
	}

	return toString(toString(name), value);
}

String HttpHeaderFields::toString(const String& name, const String& value)
{
	String s;
	s.reserve(name.length() + 2 + value.length() + 2);
	s += name;
	s += ": ";
	s += value;
	s += "\r\n";
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
		return static_cast<HttpHeaderFieldName>(unsigned(HTTP_HEADER_CUSTOM) + index);
	}

	return HTTP_HEADER_UNKNOWN;
}
