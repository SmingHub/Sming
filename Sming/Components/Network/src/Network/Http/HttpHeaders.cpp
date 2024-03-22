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

String HttpHeaders::HeaderConst::getFieldName() const
{
	return fields.toString(key());
}

size_t HttpHeaders::HeaderConst::printTo(Print& p) const
{
	size_t n{0};
	n += p.print(getFieldName());
	n += p.print(" = ");
	n += p.print(value());
	return n;
}

HttpHeaders::HeaderConst::operator String() const
{
	return fields.toString(key(), value());
}

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
	for(auto hdr : headers) {
		operator[](hdr.getFieldName()) = hdr.value();
	}
}
