/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpHeaders.cpp
 *
 ****/

#include "HttpHeaders.h"
#include <debug_progmem.h>

const String& HttpHeaders::operator[](const String& name) const
{
	auto field = fromString(name);
	if(field == HTTP_HEADER_UNKNOWN) {
		return nil;
	}
	return operator[](field);
}

bool HttpHeaders::append(const HttpHeaderFieldName& name, const String& value)
{
	int i = indexOf(name);
	if(i < 0) {
		operator[](name) = value;
		return true;
	}

	if(!getFlags(name)[Flag::Multi]) {
		debug_w("[HTTP] Append not supported for header field '%s'", toString(name).c_str());
		return false;
	}

	valueAt(i) += '\0' + value;

	return true;
}

void HttpHeaders::setMultiple(const HttpHeaders& headers)
{
	for(unsigned i = 0; i < headers.count(); i++) {
		HttpHeaderFieldName fieldName = headers.keyAt(i);
		auto fieldNameString = headers.toString(fieldName);
		operator[](fieldNameString) = headers.valueAt(i);
	}
}
