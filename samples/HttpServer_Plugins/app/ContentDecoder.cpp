/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 *
 * @author: 2021 - Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "ContentDecoder.h"

// We register two events - one is executed as soon as the client request comes to the server
// and in it we add a header to the response that inform the http client that
// we support our own content encoding called "test"
DEFINE_FSTR_LOCAL(ENCODING_NAME, "test")

bool ContentDecoder::headersComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	if(request.headers[HTTP_HEADER_CONTENT_ENCODING] == ENCODING_NAME) {
		response.headers[HTTP_HEADER_CONTENT_ENCODING] = ENCODING_NAME;
	}

	return true;
}

bool ContentDecoder::urlComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	String content = response.headers[HTTP_HEADER_ACCEPT_ENCODING];
	if(content.length() > 0) {
		content += ", ";
	}
	content += ENCODING_NAME;
	response.headers[HTTP_HEADER_ACCEPT_ENCODING] = content;

	return true;
}

bool ContentDecoder::bodyReceived(HttpServerConnection& connection, HttpRequest& request, char*& data, size_t& length)
{
	if(request.headers[HTTP_HEADER_CONTENT_ENCODING] == ENCODING_NAME) {
		for(unsigned i = 0; i < length; i++) {
			data[i]++;
		}
	}

	return true;
}
