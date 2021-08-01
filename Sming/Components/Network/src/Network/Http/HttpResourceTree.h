/****`
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpResourceTree.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "HttpResource.h"

using HttpPathDelegate = Delegate<void(HttpRequest& request, HttpResponse& response)>;

/** @brief Identifies the default resource path */
#define RESOURCE_PATH_DEFAULT String('*')

/**
 * @brief Class to map URL paths to classes which handle them
 * @ingroup http
 */
class HttpResourceTree : public ObjectMap<String, HttpResource>
{
public:
	/** @brief Set the default resource handler
	 *  @param resource The default resource handler
	 */
	HttpResource* setDefault(HttpResource* resource)
	{
		set(RESOURCE_PATH_DEFAULT, resource);
		return resource;
	}

	/** @brief Set the default resource handler, identified by "*" wildcard
	 *  @param onRequestComplete The default resource handler
	 */
	HttpResource* setDefault(const HttpResourceDelegate& onRequestComplete)
	{
		return set(RESOURCE_PATH_DEFAULT, onRequestComplete);
	}

	/** @brief Set the default resource handler, identified by "*" wildcard */
	HttpResource* setDefault(const HttpPathDelegate& callback)
	{
		return set(RESOURCE_PATH_DEFAULT, callback);
	}

	/** @brief Get the current default resource handler, if any
	 *  @retval HttpResource*
	 */
	HttpResource* getDefault()
	{
		return find(RESOURCE_PATH_DEFAULT);
	}

	using ObjectMap::set;

	/**
	 * @brief Set a callback to handle the given path
	 * @param path URL path
	 * @param onRequestComplete Delegate to handle this path
	 * @retval HttpResource* The created resource object
	 * @note Path should start with slash. Trailing slashes will be removed.
	 * @note Any existing handler for this path is replaced
	 */
	HttpResource* set(const String& path, const HttpResourceDelegate& onRequestComplete);

	/**
	 * @brief Set a callback to handle the given path, with one or more plugins
	 * @param path URL path
	 * @param onRequestComplete Delegate to handle this path
	 * @param plugin Plugins to register for the resource
	 * @retval HttpResource* The created resource object
	 * @note Path should start with slash. Trailing slashes will be removed.
	 * @note Any existing handler for this path is replaced
	 */
	template <class... Tail>
	HttpResource* set(const String& path, const HttpResourceDelegate& onRequestComplete, HttpResourcePlugin* plugin,
					  Tail... plugins)
	{
		registerPlugin(plugin, plugins...);
		auto res = set(path, onRequestComplete);
		res->addPlugin(plugin, plugins...);
		return res;
	}

	/**
	 * @brief Add a new path resource with a callback
	 * @param path URL path
	 * @param callback The callback that will handle this path
	 * @note Path should start with slash. Trailing slashes will be removed
	 * @note Any existing handler for this path is replaced
	 */
	HttpResource* set(String path, const HttpPathDelegate& callback);

	/**
	 * @brief Add a new path resource with callback and one or more  plugins
	 * @param path URL path
	 * @param callback The callback that will handle this path
	 * @param plugin - optional resource plugin
	 * @retval HttpResource* The created resource object
	 * @note Path should start with slash. Trailing slashes will be removed
	 * @note Any existing handler for this path is replaced
	 */
	template <class... Tail>
	HttpResource* set(const String& path, const HttpPathDelegate& callback, HttpResourcePlugin* plugin, Tail... plugins)
	{
		registerPlugin(plugin, plugins...);
		auto res = set(path, callback);
		res->addPlugin(plugin, plugins...);
		return res;
	}

private:
	void registerPlugin(HttpResourcePlugin* plugin)
	{
		loadedPlugins.add(plugin);
	}

	template <class... Tail> void registerPlugin(HttpResourcePlugin* plugin, Tail... plugins)
	{
		registerPlugin(plugin);
		registerPlugin(plugins...);
	}

	HttpResourcePlugin::OwnedList loadedPlugins;
};
