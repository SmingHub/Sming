/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebsocketResource.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "HttpServerConnection.h"
#include "HttpResource.h"
#include "WString.h"
#include "WHashMap.h"

typedef Delegate<void(HttpFiles&)> HttpFilesMapper;

class HttpMultipartResource : public HttpResource
{
public:
	HttpMultipartResource(const HttpFilesMapper& mapper, HttpResourceDelegate process)
	{
		onHeadersComplete = HttpResourceDelegate(&HttpMultipartResource::setFileMap, this);
		onRequestComplete = process;
		this->mapper = mapper;
	}

	virtual int setFileMap(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);

	void shutdown(HttpServerConnection& connection) override;

private:
	HttpFilesMapper mapper;
};
