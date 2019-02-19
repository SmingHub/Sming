/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpBodyParser.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpBodyParser.h"
#include "../WebHelpers/escape.h"

void formUrlParser(HttpRequest& request, const char* at, int length)
{
	auto state = static_cast<FormUrlParserState*>(request.args);

	if(length == PARSE_DATASTART) {
		delete state;
		request.args = new FormUrlParserState;
		return;
	}

	auto& params = request.postParams;

	if(length == PARSE_DATAEND) {
		// Unescape post parameters
		// @todo this should be done within the HttpParams class
		for(unsigned i = 0; i < params.count(); i++) {
			uri_unescape_inplace(params.keyAt(i));
			uri_unescape_inplace(params.valueAt(i));
		}

		delete state;
		request.args = nullptr;

		return;
	}

	if(state == nullptr) {
		debug_e("Invalid request argument");
		return;
	}

	String data = String(at, length);

	while(data.length()) {
		int pos = data.indexOf(state->searchChar);
		if(pos < 0) {
			if(state->searchChar == '=') {
				state->postName += data;
			} else {
				params[state->postName] += data;
			}

			return;
		}

		String buf = data.substring(0, pos);
		if(state->searchChar == '=') {
			state->postName += buf;
			state->searchChar = '&';
		} else {
			params[state->postName] += buf;
			state->searchChar = '=';
			state->postName = nullptr;
		}

		data.remove(0, pos + 1);
	}
}

void bodyToStringParser(HttpRequest& request, const char* at, int length)
{
	auto data = static_cast<String*>(request.args);

	if(length == PARSE_DATASTART) {
		delete data;
		data = new String();
		request.args = (void*)data;
		return;
	}

	if(!data) {
		debug_e("Invalid request argument");
		return;
	}

	if(length == PARSE_DATAEND) {
		request.setBody(*data);
		delete data;
		request.args = nullptr;
		return;
	}

	data->concat(at, length);
}
