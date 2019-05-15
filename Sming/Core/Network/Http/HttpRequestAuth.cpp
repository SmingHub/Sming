/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpRequestAuth.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpRequestAuth.h"
#include "HttpRequest.h"
#include "Network/WebHelpers/base64.h"

// Basic Auth
void HttpBasicAuth::setRequest(HttpRequest* request)
{
	request->headers[HTTP_HEADER_AUTHORIZATION] = F("Basic ") + base64_encode(username + ':' + password);
}

// Digest Auth

void HttpDigestAuth::setResponse(HttpResponse* response)
{
	if(response->code != HTTP_STATUS_UNAUTHORIZED) {
		return;
	}

	if(response->headers.contains(HTTP_HEADER_WWW_AUTHENTICATE) &&
	   response->headers[HTTP_HEADER_WWW_AUTHENTICATE].indexOf(F("Digest")) >= 0) {
		String authHeader = response->headers[HTTP_HEADER_WWW_AUTHENTICATE];
		/*
		 * Example (see: https://tools.ietf.org/html/rfc2069#page-4):
		 *
		 * WWW-Authenticate: Digest    realm="testrealm@host.com",
                            nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",
                            opaque="5ccc069c403ebaf9f0171e9517f40e41"
		 *
		 */

		// TODO: process WWW-Authenticate header

		String authResponse = F("Digest username=\"") + username + '"';
		/*
		 * Example (see: https://tools.ietf.org/html/rfc2069#page-4):
		 *
		 * Authorization: Digest       username="Mufasa",
                            realm="testrealm@host.com",
                            nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",
                            uri="/dir/index.html",
                            response="e966c932a9242554e42c8ee200cec7f6",
                            opaque="5ccc069c403ebaf9f0171e9517f40e41"
		 */

		// TODO: calculate the response...
		request->headers[HTTP_HEADER_AUTHORIZATION] = authResponse;
		request->retries = 1;
	}
}
