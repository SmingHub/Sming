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

/**
 * @brief Base plugin class. Implementations should be based on either `HttpPreFilter` or `HttpPostFilter`
 */
class HttpResourcePlugin : public LinkedObjectTemplate<HttpResourcePlugin>
{
public:
	using OwnedList = OwnedLinkedObjectListTemplate<HttpResourcePlugin>;

protected:
	friend class HttpResource;

	virtual int getPriority() const = 0;

	virtual bool urlComplete(HttpServerConnection&, HttpRequest&, HttpResponse&)
	{
		return true;
	}

	virtual bool headersComplete(HttpServerConnection&, HttpRequest&, HttpResponse&)
	{
		return true;
	}

	virtual bool upgradeReceived(HttpServerConnection&, HttpRequest&, [[maybe_unused]] char* data,
								 [[maybe_unused]] size_t length)
	{
		return true;
	}

	virtual bool bodyReceived(HttpServerConnection&, HttpRequest&, [[maybe_unused]] char*& data,
							  [[maybe_unused]] size_t& length)
	{
		return true;
	}

	virtual bool requestComplete(HttpServerConnection&, HttpRequest&, HttpResponse&)
	{
		return true;
	}
};

/**
 * @brief Filter plugins run *before* the resource is invoked
 */
class HttpPreFilter : public HttpResourcePlugin
{
private:
	int getPriority() const override
	{
		return 1;
	}
};

/**
 * @brief Filter plugins run *after* the resource is invoked
 */
class HttpPostFilter : public HttpResourcePlugin
{
private:
	int getPriority() const override
	{
		return -1;
	}
};
