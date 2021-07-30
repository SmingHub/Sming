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

#pragma once

#include <Data/LinkedObjectList.h>
#include "../HttpRequest.h"
#include "../HttpResponse.h"

class HttpServerConnection;

class HttpResourcePlugin : public LinkedObjectTemplate<HttpResourcePlugin>
{
public:
	using OwnedList = OwnedLinkedObjectListTemplate<HttpResourcePlugin>;

protected:
	friend class HttpResource;

	virtual int getPriority() const = 0;

	virtual bool urlComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
	{
		return true;
	}

	virtual bool headersComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
	{
		return true;
	}

	virtual bool upgradeReceived(HttpServerConnection& connection, HttpRequest&, char* data, size_t length)
	{
		return true;
	}

	virtual bool bodyReceived(HttpServerConnection& connection, HttpRequest& request, char*& data, size_t& length)
	{
		return true;
	}

	virtual bool requestComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
	{
		return true;
	}
};
