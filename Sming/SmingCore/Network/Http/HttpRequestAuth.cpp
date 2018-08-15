/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpRequestAuth
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpRequestAuth.h"
#include "HttpRequest.h"
#include "../Services/WebHelpers/base64.h"

/* HttpBasicAuth */

void HttpBasicAuth::setRequest(HttpRequest* request)
{
	request->headers[hhfn_Authorization] = F("Basic ") + base64_encode(_username + ":" + _password);
}

/* HttpDigestAuth */

void HttpDigestAuth::setResponse(HttpResponse* response)
{
	if (response->code != HTTP_STATUS_UNAUTHORIZED)
		return;

	const String& authHeader = response->headers[hhfn_WWWAuthenticate];
	if (authHeader.indexOf(F("Digest")) < 0)
		return;

	/*
	 * Example (see: https://tools.ietf.org/html/rfc2069#page-4):
	 *
	 * WWW-Authenticate: Digest    realm="testrealm@host.com",
						nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",
						opaque="5ccc069c403ebaf9f0171e9517f40e41"
	 *
	 */

	// @todo: process WWW-Authenticate header

	String authResponse = F("Digest username=\"") + _username + "\"";
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

	// @todo: calculate the response...
	_request->headers[hhfn_Authorization] = authResponse;
	_request->retries = 1;
}
