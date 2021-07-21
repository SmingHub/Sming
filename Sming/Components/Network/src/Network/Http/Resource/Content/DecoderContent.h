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

class DecoderContent : public HttpResourcePlugin
{
	bool registerPlugin(HttpEventedResource& resource) override
	{
		// Check if the provided content is encoded in some way
		return resource.addEvent(
			HttpEventedResource::EVENT_BODY,
			[](HttpServerConnection& connection, char** at, int* length) {
				auto request = connection.getRequest();
				if(request->headers[HTTP_HEADER_CONTENT_ENCODING] == "test") {
					char* data = *at;
					for(unsigned i = 0; i < *length; i++) {
						data[i]++;
					}
				}

				return true;
			},
			1);
	}
};
