#include "HttpResource.h"
#include "HttpServerConnection.h"

namespace
{
// Sequence to mark failed request, normally never occurs in headers
constexpr char SKIP_HEADER[]{2, 1, 0};

bool shouldSkip(const HttpRequest& request)
{
	return request.headers[SKIP_HEADER] == "1";
}

int requestFailed(HttpRequest& request, int err)
{
	debug_e("REQUEST FAILED");
	request.headers[SKIP_HEADER] = "1";
	return err;
}

} // namespace

#define FUNCTION_TEMPLATE(delegate, method, ...)                                                                       \
	if(shouldSkip(request)) {                                                                                          \
		return 0;                                                                                                      \
	}                                                                                                                  \
                                                                                                                       \
	bool resourceHandled = !delegate;                                                                                  \
	for(auto& plugin : plugins) {                                                                                      \
		if(!resourceHandled && plugin->getPriority() == 0) {                                                           \
			int err = delegate(connection, request, ##__VA_ARGS__);                                                    \
			if(err != 0) {                                                                                             \
				return requestFailed(request, err);                                                                    \
			}                                                                                                          \
			resourceHandled = true;                                                                                    \
		}                                                                                                              \
		if(!plugin->method(connection, request, ##__VA_ARGS__)) {                                                      \
			return requestFailed(request, 0);                                                                          \
		}                                                                                                              \
	}                                                                                                                  \
	return resourceHandled ? 0 : delegate(connection, request, ##__VA_ARGS__);

int HttpResource::handleUrl(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	FUNCTION_TEMPLATE(onUrlComplete, urlComplete, response)
}

int HttpResource::handleHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	FUNCTION_TEMPLATE(onHeadersComplete, headersComplete, response)
}

int HttpResource::handleUpgrade(HttpServerConnection& connection, HttpRequest& request, char* data, size_t length)
{
	FUNCTION_TEMPLATE(onUpgrade, upgradeReceived, data, length)
}

int HttpResource::handleBody(HttpServerConnection& connection, HttpRequest& request, char*& data, size_t& length)
{
	FUNCTION_TEMPLATE(onBody, bodyReceived, data, length)
}

int HttpResource::handleRequest(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	FUNCTION_TEMPLATE(onRequestComplete, requestComplete, response)
}

void HttpResource::addPlugin(HttpResourcePlugin* plugin)
{
	if(plugin == nullptr) {
		return;
	}

	auto ref = new PluginRef{plugin};
	auto priority = plugin->getPriority();
	auto current = plugins.head();
	if(current == nullptr || (*current)->getPriority() < priority) {
		plugins.insert(ref);
		return;
	}

	while(current->next() != nullptr && (*current->getNext())->getPriority() > priority) {
		current = current->getNext();
	}

	ref->insertAfter(current);
}
