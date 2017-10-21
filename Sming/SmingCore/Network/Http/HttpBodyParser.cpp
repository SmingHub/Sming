/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpBodyParser
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpBodyParser.h"
#include "../WebHelpers/escape.h"

void formUrlParser(HttpRequest& request, const char *at, int length)
{
	FormUrlParserState* state = (FormUrlParserState*)request.args;

	if(length == -1) {
		if(state != NULL) {
			delete state;
		}
		state = new FormUrlParserState;
		request.args = (void *)state;
		return;
	}

	if(length == -2) {
		int maxLength = 0;
		for(int i=0; i<request.postParams.count(); i++) {
			int kLength = request.postParams.keyAt(i).length();
			int vLength = request.postParams.valueAt(i).length();
			if(maxLength < vLength || maxLength < kLength) {
				maxLength = (kLength < vLength ? vLength : kLength);
			}
		}

		char* buffer = new char[maxLength + 1];
		for(int i=0, max = request.postParams.count(); i< max; i++) {
			String key = request.postParams.keyAt(i);
			String value = request.postParams.valueAt(i);

			uri_unescape(buffer, maxLength + 1, key.c_str(), key.length());
			String newKey = buffer;

			if(newKey != key) {
				request.postParams.remove(key);
			}

			uri_unescape(buffer, maxLength + 1, value.c_str(), value.length());
			request.postParams[newKey] = buffer;
		}
		delete[] buffer;

		if(state != NULL) {
			delete state;
			request.args = NULL;
		}

		return;
	}

	if(state == NULL) {
		debugf("Invalid request argument");
		return;
	}

	String data = String(at, length);

	while(data.length()) {
		int pos = data.indexOf(state->searchChar);
		if(pos == -1) {
			if(state->searchChar == '=') {
				state->postName += data;
			}
			else {
				request.postParams[state->postName] += data;
			}

			return;
		}

		String buf = data.substring(0, pos);
		if(state->searchChar == '=') {
			state->postName += buf;
			state->searchChar = '&';
		}
		else {
			request.postParams[state->postName] += buf;
			state->searchChar = '=';
			state->postName = "";
		}

		data = data.substring(pos + 1);
	}
}

void bodyToStringParser(HttpRequest& request, const char *at, int length)
{
	String* data = static_cast<String *>(request.args);

	if(length == -1) {
		delete data;
		data = new String();
		request.args = (void *)data;
		return;
	}

	if(length == -2) {
		request.setBody(*data);
		delete data;
		request.args = NULL;
		return;
	}

	*data += String(at, length);
}
