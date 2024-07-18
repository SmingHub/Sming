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

class ResourceBasicAuth : public HttpPreFilter
{
public:
	ResourceBasicAuth(const String& realm, const String& username, const String& password)
		: realm(realm), username(username), password(password)
	{
	}

	bool headersComplete(HttpServerConnection&, HttpRequest& request, HttpResponse& response) override
	{
		auto& headers = request.headers;
		auto authorization = headers[HTTP_HEADER_AUTHORIZATION];
		if(authorization) {
			// check the authorization
			authorization.trim();
			auto pos = authorization.indexOf(' ');
			if(pos < 0) {
				debug_w("Invalid authorization header");
				return true;
			}

			auto type = authorization.substring(0, pos);
			auto token = authorization.substring(pos + 1, authorization.length());
			if(!type.equalsIgnoreCase(F("Basic"))) {
				return true;
			}

			String text = base64_decode(token.c_str(), token.length());
			pos = text.indexOf(':');
			if(pos > 0) {
				auto providedUsername = text.substring(0, pos);
				auto providedPassword = text.substring(pos + 1, text.length());
				if(providedUsername == username && providedPassword == password) {
					return true;
				}
			}
		}

		// specify that the resource is protected...
		response.code = HTTP_STATUS_UNAUTHORIZED;
		response.headers[HTTP_HEADER_WWW_AUTHENTICATE] = F("Basic realm=\"") + realm + "\"";

		return false;
	}

private:
	String realm;
	String username;
	String password;
};
