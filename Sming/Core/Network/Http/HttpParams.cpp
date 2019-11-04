/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpParams.cpp
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "HttpParams.h"
#include "Network/WebHelpers/escape.h"
#include "Print.h"
#include "libyuarel/yuarel.h"

// Set a reasonable limit on the number of expected parameters in a query string
static const unsigned MAX_PARAMS = 16;

void HttpParams::parseQuery(char* query)
{
	clear();

	if(query == nullptr) {
		return;
	}
	// Accept query strings with or without '?' prefix
	if(*query == '?') {
		++query;
	}

	struct yuarel_param params[MAX_PARAMS];
	int paramCount = yuarel_parse_query(query, '&', params, MAX_PARAMS);
	if(paramCount <= 0) {
		return;
	}

	allocate(paramCount);
	if(keys == nullptr || values == nullptr) {
		// Allocation failure
		return;
	}

	for(int i = 0; i < paramCount; ++i) {
		String key = uri_unescape_inplace(params[i].key);
		String value = uri_unescape_inplace(params[i].val);
		operator[](key) = value;
	}
}

String HttpParams::toString() const
{
	if(count() == 0) {
		return nullptr;
	}

	/*

		The following code is an optimised version of this:

			String result;
			for (unsigned i = 0; i < count(); ++i) {
				if (i > 0)
					result += '&';
				result += uri_escape(keyAt(i));
				result += '=';
				result += uri_escape(valueAt(i));
			}

	 */

	// Calculate length of result so we can reserve result String memory in one go
	unsigned resultLength = 0;
	for(unsigned i = 0; i < count(); ++i) {
		resultLength += uri_escape_len(keyAt(i)) + 1 + uri_escape_len(valueAt(i)) + 1; // Allow for '?', '=' and '&'
	}

	String result;
	if(!result.setLength(resultLength)) {
		return nullptr;
	}

	// Write out the string content
	char* resultPtr = result.begin();
	*resultPtr++ = '?';
	for(unsigned i = 0; i < count(); ++i) {
		if(i > 0) {
			*resultPtr++ = '&';
		}

		// result += uri_escape(keyAt(i));
		const String& key = keyAt(i);
		uri_escape(resultPtr, 1 + result.end() - resultPtr, key.c_str(), key.length());
		resultPtr += strlen(resultPtr);

		// result += '=';
		*resultPtr++ = '=';

		// result += uri_escape(valueAt(i));
		const String& value = valueAt(i);
		uri_escape(resultPtr, 1 + result.end() - resultPtr, value.c_str(), value.length());
		resultPtr += strlen(resultPtr);
	}

	assert(resultPtr == result.end());

	return result;
}

size_t HttpParams::printTo(Print& p) const
{
	size_t charsPrinted = 0;
	for(unsigned i = 0; i < count(); i++) {
		if(i > 0) {
			charsPrinted += p.print('&');
		}
		charsPrinted += p.print(uri_escape(keyAt(i)));
		charsPrinted += p.print('=');
		charsPrinted += p.print(uri_escape(valueAt(i)));
	}

	return charsPrinted;
}

void HttpParams::debugPrintTo(Print& p) const
{
	for(unsigned i = 0; i < count(); i++) {
		p.print(' ');
		p.print(keyAt(i));
		p.print(_F(" = ["));
		p.print(valueAt(i));
		p.println(']');
	}
}
