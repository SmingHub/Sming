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

#include <WString.h>
#include <Data/ObjectMap.h>

#include "Resource/HttpResourcePlugin.h"

class HttpServerConnection;

using HttpServerConnectionBodyDelegate =
	Delegate<int(HttpServerConnection& connection, HttpRequest&, const char* at, int length)>;
using HttpServerConnectionUpgradeDelegate =
	Delegate<int(HttpServerConnection& connection, HttpRequest&, char* at, int length)>;
using HttpResourceDelegate =
	Delegate<int(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)>;

/**
 * @brief Instances of this class are registered with an HttpServer for a specific URL
 * @ingroup httpserver
 */
class HttpResource
{
public:
	virtual ~HttpResource() = default;

	/**
	 * @brief Takes care to cleanup the connection
	 */
	virtual void shutdown(HttpServerConnection&)
	{
	}

public:
	class PluginRef : public LinkedObjectTemplate<PluginRef>
	{
	public:
		using OwnedList = OwnedLinkedObjectListTemplate<PluginRef>;

		PluginRef(HttpResourcePlugin* plugin) : plugin(plugin)
		{
		}

		HttpResourcePlugin* operator->() const
		{
			return plugin;
		}

	private:
		HttpResourcePlugin* plugin;
	};

	HttpResourceDelegate onUrlComplete = nullptr;			 ///< URL is ready. Path and status code are available
	HttpServerConnectionBodyDelegate onBody = nullptr;		 ///< resource wants to process the raw body data
	HttpResourceDelegate onHeadersComplete = nullptr;		 ///< headers are ready
	HttpResourceDelegate onRequestComplete = nullptr;		 ///< request is complete OR upgraded
	HttpServerConnectionUpgradeDelegate onUpgrade = nullptr; ///< request is upgraded and raw data is passed to it

	void addPlugin(HttpResourcePlugin* plugin);

	template <class... Tail> void addPlugin(HttpResourcePlugin* plugin, Tail... plugins)
	{
		addPlugin(plugin);
		addPlugin(plugins...);
	}

private:
	friend class HttpServerConnection;

	PluginRef::OwnedList plugins;

	int handleUrl(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);
	int handleHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);
	int handleUpgrade(HttpServerConnection& connection, HttpRequest& request, char* data, size_t length);
	int handleBody(HttpServerConnection& connection, HttpRequest& request, char*& data, size_t& length);
	int handleRequest(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);
};
