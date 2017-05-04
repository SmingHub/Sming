/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpResource
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTP_RESOURCE_H_
#define _SMING_CORE_HTTP_RESOURCE_H_

#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"
#include "../../Delegate.h"

#include "HttpResponse.h"
#include "HttpRequest.h"

class HttpServerConnection;

typedef Delegate<int(HttpServerConnection& connection, HttpRequest&, const char *at, int length)> HttpServerConnectionBodyDelegate;
typedef Delegate<int(HttpServerConnection& connection, HttpRequest&, char *at, int length)> HttpServerConnectionUpgradeDelegate;
typedef Delegate<int(HttpServerConnection&, HttpRequest&, HttpResponse&)> HttpResourceDelegate;
typedef Delegate<void(HttpRequest&, HttpResponse&)> HttpPathDelegate; // << deprecated

class HttpResource {
public:
	virtual ~HttpResource() {}

public:
	HttpServerConnectionBodyDelegate onBody = 0; // << called when the resource wants to process the raw body data
	HttpResourceDelegate onHeadersComplete = 0; // << called when the headers are ready
	HttpResourceDelegate onRequestComplete = 0; // << called when the request is complete OR upgraded
	HttpServerConnectionUpgradeDelegate onUpgrade = 0; // called when the request is upgraded and raw data is passed to it
};

class HttpCompatResource: public HttpResource {
public:
	HttpCompatResource(const HttpPathDelegate& callback);

private:
	int requestComplete(HttpServerConnection&, HttpRequest& , HttpResponse& );

private:
	HttpPathDelegate callback;
};


typedef HashMap<String, HttpResource*> ResourceTree;

#endif /* _SMING_CORE_HTTP_RESOURCE_H_ */
