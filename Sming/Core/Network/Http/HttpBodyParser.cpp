/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpBodyParser.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 * 	Original author
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 * 	Implemented un-escaping of incoming parameters
 *
 ****/

#include "HttpBodyParser.h"
#include "Network/WebHelpers/escape.h"

/*
 * Content is received in chunks which we need to reassemble into name=value pairs.
 * This structure stores the temporary values during parsing.
 */
typedef struct {
	char searchChar = '=';
	String postName;
	String postValue;
} FormUrlParserState;

/*
 * The incoming URL is parsed
 */
void formUrlParser(HttpRequest& request, const char* at, int length)
{
	auto state = static_cast<FormUrlParserState*>(request.args);

	if(length == PARSE_DATASTART) {
		delete state;
		request.args = new FormUrlParserState;
		return;
	}

	assert(state != nullptr);

	auto& params = request.postParams;

	if(length == PARSE_DATAEND) {
		// Store last parameter, if there is one
		if(state->postName.length() != 0) {
			uri_unescape_inplace(state->postValue);
			params[state->postName] = state->postValue;
		}

		delete state;
		request.args = nullptr;

		return;
	}

	if(state == nullptr) {
		debug_e("Invalid request argument");
		return;
	}

	while(length > 0) {
		// Look for search character ('=' or '&') in received text
		auto found = static_cast<const char*>(memchr(at, state->searchChar, length));
		unsigned foundLength = (found == nullptr) ? length : (found - at);

		if(foundLength != 0) {
			if(state->searchChar == '=') {
				state->postName.concat(at, foundLength);
			} else {
				state->postValue.concat(at, foundLength);
			}
		}

		if(found == nullptr) {
			break;
		}

		if(state->searchChar == '=') {
			uri_unescape_inplace(state->postName);
			state->searchChar = '&';
		} else {
			uri_unescape_inplace(state->postValue);
			params[state->postName] = state->postValue;
			state->searchChar = '=';
			// Keep String memory allocated, but clear content
			state->postName.setLength(0);
			state->postValue.setLength(0);
		}
		++foundLength; // Skip the '=' or '&'
		at += foundLength;
		length -= foundLength;
	}
}

void bodyToStringParser(HttpRequest& request, const char* at, int length)
{
	auto data = static_cast<String*>(request.args);

	if(length == PARSE_DATASTART) {
		delete data;
		data = new String();
		request.args = data;
		return;
	}

	if(data == nullptr) {
		debug_e("Invalid request argument");
		return;
	}

	if(length == PARSE_DATAEND || length < 0) {
		request.setBody(*data);
		delete data;
		request.args = nullptr;
		return;
	}

	data->concat(at, length);
}
