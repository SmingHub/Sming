/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpResource.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "WString.h"
#include "Data/ObjectMap.h"

#include "HttpResponse.h"
#include "HttpRequest.h"

class HttpServerConnection;

using HttpServerConnectionBodyDelegate =
	Delegate<int(HttpServerConnection& connection, HttpRequest&, const char* at, int length)>;
using HttpServerConnectionUpgradeDelegate =
	Delegate<int(HttpServerConnection& connection, HttpRequest&, char* at, int length)>;
using HttpResourceDelegate =
	Delegate<int(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)>;

/**
 * @brief Instances of this class are registered with an HttpServer for a specific URL
 * @ingroup http
 */
class HttpResource
{
public:
	virtual ~HttpResource()
	{
	}

	/**
	 * @brief Takes care to cleanup the connection
	 */
	virtual void shutdown(HttpServerConnection& connection)
	{
	}

public:
	HttpServerConnectionBodyDelegate onBody = nullptr;		 ///< resource wants to process the raw body data
	HttpResourceDelegate onHeadersComplete = nullptr;		 ///< headers are ready
	HttpResourceDelegate onRequestComplete = nullptr;		 ///< request is complete OR upgraded
	HttpServerConnectionUpgradeDelegate onUpgrade = nullptr; ///< request is upgraded and raw data is passed to it
};
