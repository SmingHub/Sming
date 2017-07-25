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

#ifndef _SMING_CORE_HTTP_REQUEST_AUTH_H_
#define _SMING_CORE_HTTP_REQUEST_AUTH_H_

#include "HttpResponse.h"

class HttpRequest;

class AuthAdapter {
public:
	virtual void setRequest(HttpRequest* request) = 0;

	__forceinline virtual void setResponse(HttpResponse *response) {
		return;
	}

	virtual ~AuthAdapter() {}
};


class HttpBasicAuth: public AuthAdapter {
public:
	HttpBasicAuth(const String& username, const String& password);

	void setRequest(HttpRequest* request);
private:
	String username;
	String password;
};

class HttpDigestAuth: public AuthAdapter {
public:
	HttpDigestAuth(const String& username, const String& password);

	void setRequest(HttpRequest* request);

	void setResponse(HttpResponse *response);

private:
	String username;
	String password;
	HttpRequest* request=NULL;
};

#endif /* _SMING_CORE_HTTP_REQUEST_AUTH_H_ */
