#include "HttpEventedResource.h"
#include "../HttpServerConnection.h"

static constexpr char SKIP_HEADER[]{2, 1, 0};

HttpEventedResource::HttpEventedResource(HttpResource* resource)
{
	delegate = resource;

	// [ Register the main events with priority 0 ]
	addEvent(EventType::EVENT_URL, [this](HttpServerConnection& connection, char** at = nullptr,
										  int* length = nullptr) {
		if(delegate->onUrlComplete) {
			auto hasError = delegate->onUrlComplete(connection, *connection.getRequest(), *connection.getResponse());
			if(hasError) {
				return false;
			}
		}

		return true;
	});

	addEvent(EventType::EVENT_HEADERS,
			 [this](HttpServerConnection& connection, char** at = nullptr, int* length = nullptr) {
				 if(delegate->onHeadersComplete) {
					 auto hasError =
						 delegate->onHeadersComplete(connection, *connection.getRequest(), *connection.getResponse());
					 if(hasError) {
						 return false;
					 }
				 }

				 return true;
			 });

	addEvent(EventType::EVENT_UPGRADE,
			 [this](HttpServerConnection& connection, char** at = nullptr, int* length = nullptr) {
				 if(delegate->onUpgrade) {
					 auto hasError = delegate->onUpgrade(connection, *connection.getRequest(), *at, *length);
					 if(hasError) {
						 return false;
					 }
				 }

				 return true;
			 });

	addEvent(EventType::EVENT_BODY,
			 [this](HttpServerConnection& connection, char** at = nullptr, int* length = nullptr) {
				 if(delegate->onBody) {
					 auto hasError = delegate->onBody(connection, *connection.getRequest(), at, length);
					 if(hasError) {
						 return false;
					 }
				 }

				 return true;
			 });

	addEvent(EventType::EVENT_COMPLETE,
			 [this](HttpServerConnection& connection, char** at = nullptr, int* length = nullptr) {
				 if(delegate->onRequestComplete) {
					 auto hasError =
						 delegate->onRequestComplete(connection, *connection.getRequest(), *connection.getResponse());
					 if(hasError) {
						 return false;
					 }
				 }

				 return true;
			 });

	// [ Register the resource callbacks ]
	onUrlComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
		return runEvent(EventType::EVENT_URL, connection, nullptr, 0);
	};

	onHeadersComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
		return runEvent(EventType::EVENT_HEADERS, connection, nullptr, 0);
	};

	onUpgrade = [this](HttpServerConnection& connection, HttpRequest& request, char* at, int length) -> int {
		return runEvent(EventType::EVENT_UPGRADE, connection, nullptr, 0);
	};

	onBody = [this](HttpServerConnection& connection, HttpRequest& request, char** at, int* length) -> int {
		return runEvent(EventType::EVENT_BODY, connection, at, length);
	};

	onRequestComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
		return runEvent(EventType::EVENT_COMPLETE, connection, nullptr, 0);
	};
}

int HttpEventedResource::runEvent(EventType type, HttpServerConnection& connection, char** at, int* length)
{
	auto request = connection.getRequest();
	if(request->headers[SKIP_HEADER] == "1") {
		// skip this event
		return 0;
	}

	auto list = getEvents(type);
	if(list != nullptr) {
		for(auto& current : *list) {
			bool success = current.data(connection, at, length);
			if(!success) {
				request->headers[SKIP_HEADER] = "1";
				break;
			}
		}
	}

	return 0;
}

bool HttpEventedResource::addEvent(EventType type, EventCallback callback, int priority)
{
	auto list = events[type];
	if(!list) {
		list = new EventList();
	}

	return list->add(callback, priority);
}
