#include "App.h"
#include "Client.h"

namespace Dial
{
bool App::status(ResponseCallback onResponse)
{
	auto request = new HttpRequest(getApplicationUrl());
	request->method = HTTP_GET;
	if(onResponse) {
		request->onRequestComplete([onResponse, this](HttpConnection& connection, bool successful) -> int {
			onResponse(*this, *(connection.getResponse()));

			return 0;
		});
	}

	return client.sendRequest(request);
}

bool App::sendRunRequest(HttpRequest* request, ResponseCallback onResponse)
{
	request->method = HTTP_POST;
	request->onRequestComplete([onResponse, this](HttpConnection& connection, bool successful) -> int {
		auto headers = connection.getResponse()->headers;
		if(headers.contains(HTTP_HEADER_LOCATION)) {
			this->instanceUrl = headers[HTTP_HEADER_LOCATION];
		}

		if(onResponse != nullptr) {
			onResponse(*this, *(connection.getResponse()));
		}

		return 0;
	});

	return client.sendRequest(request);
}

bool App::run(ResponseCallback onResponse)
{
	auto request = new HttpRequest(getApplicationUrl());
	return sendRunRequest(request, onResponse);
}

bool App::run(const String& body, MimeType mime, ResponseCallback onResponse)
{
	auto request = new HttpRequest(getApplicationUrl());
	request->headers[HTTP_HEADER_CONTENT_TYPE] = toString(mime);
	if(body.length() != 0) {
		request->setBody(body);
	}

	return sendRunRequest(request, onResponse);
}

bool App::run(const HttpParams& params, ResponseCallback onResponse)
{
	auto request = new HttpRequest(getApplicationUrl());
	request->postParams = params;
	return sendRunRequest(request, onResponse);
}

bool App::stop(ResponseCallback onResponse)
{
	if(instanceUrl.length() == 0) {
		debug_w("Instance URL not set. Only started apps can be stopped.");
		return false;
	}

	auto request = new HttpRequest(Url(instanceUrl));
	request->method = HTTP_DELETE;
	if(onResponse != nullptr) {
		request->onRequestComplete([onResponse, this](HttpConnection& connection, bool successful) -> int {
			onResponse(*this, *(connection.getResponse()));

			return 0;
		});
	}

	return client.sendRequest(request);
}

} // namespace Dial
