/****`
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpResourceTree.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpResourceTree.h"

class HttpCompatResource : public HttpResource
{
public:
	explicit HttpCompatResource(const HttpPathDelegate& callback) : callback(callback)
	{
		onRequestComplete = HttpResourceDelegate(&HttpCompatResource::requestComplete, this);
	}

private:
	int requestComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
	{
		callback(request, response);
		return 0;
	}

private:
	HttpPathDelegate callback;
};

/* HttpResourceTree */

void HttpResourceTree::set(const String& path, HttpResource* resource, HttpResourcePlugin* plugin)
{
	if(resource == nullptr) {
		return;
	}

	HttpResource* oldResource = get(path);
	bool replaceResource = (oldResource != resource);

	if(plugin != nullptr) {
		if(resource->getType() != HttpResource::EVENTED_RESOURCE) {
			resource = new HttpEventedResource(resource);
		}

		plugin->registerPlugin(*(static_cast<HttpEventedResource*>(resource)));
		loadedPlugins.add(plugin);
	}

	if(replaceResource) {
		set(path, resource);
	}
}

void HttpResourceTree::set(const String& path, const HttpResourceDelegate& onRequestComplete,
						   HttpResourcePlugin* plugin)
{
	auto resource = get(path);
	if(resource == nullptr) {
		debug_i("'%s' registered", path.c_str());
		resource = new HttpResource;
		resource->onRequestComplete = onRequestComplete;
	}

	set(path, resource, plugin);
}

void HttpResourceTree::set(String path, const HttpPathDelegate& callback, HttpResourcePlugin* plugin)
{
	if(path.length() > 1 && path.endsWith("/")) {
		path.remove(path.length() - 1);
	}

	HttpResource* resource = get(path);
	if(resource == nullptr) {
		debug_i("'%s' registered", path.c_str());

		resource = new HttpCompatResource(callback);
	}

	set(path, resource, plugin);
}
