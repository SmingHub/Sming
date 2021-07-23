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

class ContentDecoderPlugin : public HttpResourcePlugin
{
public:
	bool registerPlugin(HttpEventedResource& resource) override;

	bool onHeaders(HttpServerConnection& connection, char** at, int* length);

	bool onUrl(HttpServerConnection& connection, char** at, int* length);

	bool onBody(HttpServerConnection& connection, char** at, int* length);
};
