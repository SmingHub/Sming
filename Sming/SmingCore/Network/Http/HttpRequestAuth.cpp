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
#include "../../Services/WebHelpers/base64.h"

HttpBasicAuth::HttpBasicAuth(const String& username, const String& password) {
	this->username = username;
	this->password = password;
}

// Basic Auth
void HttpBasicAuth::setRequest(HttpRequest* request) {
	String clearText = username+":" + password;
	int hashLength = clearText.length() * 4;
	char hash[hashLength];
	base64_encode(clearText.length(), (const unsigned char *)clearText.c_str(), hashLength, hash);

	request->setHeader("Authorization", "Basic "+ String(hash));
}

// Digest Auth
HttpDigestAuth::HttpDigestAuth(const String& username, const String& password) {
	this->username = username;
	this->password = password;
}

void HttpDigestAuth::setRequest(HttpRequest* request) {
	this->request = request;
}

void HttpDigestAuth::setResponse(HttpResponse *response) {
	if(response->code != HTTP_STATUS_UNAUTHORIZED) {
		return;
	}

	if(response->headers.contains("WWW-Authenticate") && response->headers["WWW-Authenticate"].indexOf("Digest")!=-1) {
		String authHeader = response->headers["WWW-Authenticate"];
		/*
		 * Example (see: https://tools.ietf.org/html/rfc2069#page-4):
		 *
		 * WWW-Authenticate: Digest    realm="testrealm@host.com",
                            nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",
                            opaque="5ccc069c403ebaf9f0171e9517f40e41"
		 *
		 */

		// TODO: process WWW-Authenticate header

		String authResponse = "Digest username=\"" + username + "\"";
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
		request->setHeader("Authorization", authResponse);
		request->retries = 1;
	}
}

