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

#include "../HttpResourcePlugin.h"
#include <Data/WebHelpers/base64.h>

class ResourceIpAuth : public HttpPreFilter
{
public:
	ResourceIpAuth(IpAddress ip, IpAddress netmask) : ip(ip), netmask(netmask)
	{
	}

	bool urlComplete(HttpServerConnection& connection, HttpRequest&, HttpResponse& response) override
	{
		auto remoteIp = connection.getRemoteIp();
		if(remoteIp.compare(ip, netmask)) {
			// This IP is allowed to proceed
			return true;
		}

		// specify that the resource is protected...
		response.code = HTTP_STATUS_UNAUTHORIZED;
		return false;
	}

private:
	IpAddress ip;
	IpAddress netmask;
};
