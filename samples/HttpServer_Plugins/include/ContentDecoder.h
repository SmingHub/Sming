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

#include <Network/Http/HttpResource.h>

class ContentDecoder : public HttpPreFilter
{
public:
	bool headersComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) override;
	bool urlComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) override;
	bool bodyReceived(HttpServerConnection& connection, HttpRequest& request, char*& data, size_t& length) override;
};
