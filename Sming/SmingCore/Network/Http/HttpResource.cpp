/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpServerResource
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpResource.h"

HttpCompatResource::HttpCompatResource(const HttpPathDelegate& callback)
{
	this->callback = callback;
	onRequestComplete = HttpResourceDelegate(&HttpCompatResource::requestComplete, this);
}

int HttpCompatResource::requestComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) {
	callback(request, response);
	return 0;
}
