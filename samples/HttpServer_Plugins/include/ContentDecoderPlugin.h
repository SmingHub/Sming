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

#include <Network/Http/Resource/HttpResourcePlugin.h>
#include <Data/WebHelpers/base64.h>

namespace
{
constexpr char ENCODING_NAME[] = "test";
}

class ContentDecoderPlugin : public HttpResourcePlugin
{
public:
	bool registerPlugin(HttpEventedResource& resource) override
	{
		// We register two events - one is executed as soon as the client request comes to the server
		// and in it we add a header to the response that inform the http client that
		// we support our own content encoding called "test"
		resource.addEvent(
			HttpEventedResource::EVENT_URL,											// event to listen to
			HttpEventedResource::EventCallback(&ContentDecoderPlugin::onUrl, this), // callback to execute
			1); // priority to use. 1 means that this callback will be called BEFORE the normal onUrl callback

		resource.addEvent(HttpEventedResource::EVENT_HEADERS,
						  HttpEventedResource::EventCallback(&ContentDecoderPlugin::onHeaders, this),
						  1); //

		resource.addEvent(HttpEventedResource::EVENT_BODY,
						  HttpEventedResource::EventCallback(&ContentDecoderPlugin::onBody, this),
						  1); //

		return true;
	}

	bool onHeaders(HttpServerConnection& connection, char** at, int* length)
	{
		auto request = connection.getRequest();
		if(request->headers[HTTP_HEADER_CONTENT_ENCODING] == ENCODING_NAME) {
			auto response = connection.getResponse();
			response->headers[HTTP_HEADER_CONTENT_ENCODING] = ENCODING_NAME;
		}

		return true;
	}

	bool onUrl(HttpServerConnection& connection, char** at, int* length)
	{
		auto response = connection.getResponse();
		String content = response->headers["Accept-Encoding"];
		if(content.length() > 0) {
			content += ", ";
		}
		content += ENCODING_NAME;
		response->headers["Accept-Encoding"] = content;

		return true;
	}

	bool onBody(HttpServerConnection& connection, char** at, int* length)
	{
		auto request = connection.getRequest();
		if(request->headers[HTTP_HEADER_CONTENT_ENCODING] == ENCODING_NAME) {
			char* data = *at;
			for(unsigned i = 0; i < *length; i++) {
				data[i]++;
			}
		}

		return true;
	}
};
