#include "HttpEventedResource.h"
#include "../HttpServerConnection.h"

HttpEventedResource::HttpEventedResource(HttpResource* resource)
{
	delegate = resource;

	// [ Register the main events with priority 0 ]
	addEvent(EventType::EVENT_URL, [this](HttpServerConnection& connection, const char* at = nullptr, int length = 0) {
		if(delegate->onUrlComplete) {
			return delegate->onUrlComplete(connection, *connection.getRequest(), *connection.getResponse());
		}

		return 0;
	});

	addEvent(EventType::EVENT_HEADERS, [this](HttpServerConnection& connection, const char* at = nullptr, int length = 0) {
		if(delegate->onHeadersComplete) {
			return delegate->onHeadersComplete(connection, *connection.getRequest(), *connection.getResponse());
		}

		return 0;
	});

	addEvent(EventType::EVENT_UPGRADE, [this](HttpServerConnection& connection, const char* at = nullptr, int length = 0) {
		if(delegate->onUpgrade) {
			return delegate->onUpgrade(connection, *connection.getRequest(), const_cast<char*>(at), length);
		}

		return 0;
	});

	addEvent(EventType::EVENT_BODY, [this](HttpServerConnection& connection, const char* at = nullptr, int length = 0) {
		if(delegate->onBody) {
			return delegate->onBody(connection, *connection.getRequest(), at, length);
		}

		return 0;
	});


	addEvent(EventType::EVENT_COMPLETE, [this](HttpServerConnection& connection, const char* at = nullptr, int length = 0) {
		if(delegate->onRequestComplete) {
			return delegate->onRequestComplete(connection, *connection.getRequest(), *connection.getResponse());
		}

		return 0;
	});


	// [ Register the resource callbacks ]
	onUrlComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
		return runEvent(EventType::EVENT_URL, connection, nullptr, 0, true);
	};

	onHeadersComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
		return runEvent(EventType::EVENT_HEADERS, connection, nullptr, 0, true);
	};

	onUpgrade = [this](HttpServerConnection& connection, HttpRequest& request, char* at, int length) -> int {
		return runEvent(EventType::EVENT_UPGRADE, connection, nullptr, 0, false);
	};

	onBody = [this](HttpServerConnection& connection, HttpRequest& request, const char* at, int length) -> int {
		return runEvent(EventType::EVENT_BODY, connection, at, length, true);
	};

	onRequestComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
		return runEvent(EventType::EVENT_COMPLETE, connection, nullptr, 0, false);
	};
}

int HttpEventedResource::runEvent(EventType type, HttpServerConnection& connection, const char* at, int length, bool skip)
{
	auto request = connection.getRequest();
	if(skip && request->args) {
		// skip this event
		return 0;
	}

	auto list = getEvents(type);
	if(list != nullptr) {
		auto start = list->getHead();
		while(start != nullptr) {
			bool success = start->data(connection, at, length);
			if(!success) {
				request->args = (void*)1;
				break;
			}
			start = start->next;
		}
	}

	return 0;
}

bool HttpEventedResource::addEvent(EventType type, EventCallback callback, int priority)
{
	PriorityNodeList<EventCallback>* list = events[type];
	if(list == nullptr) {
		list = new PriorityNodeList<EventCallback>();
		events[type] = list;
	}

	return list->add(callback, priority);
}
