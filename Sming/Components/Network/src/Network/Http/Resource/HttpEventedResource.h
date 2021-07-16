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

#include "../HttpResource.h"
#include "../HttpResourceTree.h"
#include <Delegate.h>
#include <Data/LinkedObjectList.h>



template <typename ObjectType>
class Node
{
public:
	ObjectType data;
	int priority;
	Node* next;
};


template <typename ObjectType>
class NodeList
{
public:
	bool add(ObjectType object, int priority)
	{
		auto node = new Node<ObjectType>;
		if(node == nullptr) {
			return false;
		}

		node->data = object;
		node->priority = priority;

		if(head == nullptr) {
			head = node;
			return true;
		}

		auto start = head;
		if(start->priority < node->priority) {
			node->next = start;
			start->next = node;
			return  true;
		}

		while(start->next != nullptr && start->next->priority > priority) {
			start = start->next;
		}

		node->next = start->next;
		start->next = node;

		return true;
	}

	Node<ObjectType>* getHead() const
	{
		return head;
	}


private:
	Node<ObjectType>* head = nullptr;
};


class HttpEventedResource : public HttpResource
{
public:
	static constexpr int SKIP_REQUEST = -9999;

	enum EventType {
		EVENT_PATH,
		EVENT_HEADERS,
		EVENT_BODY,
		EVENT_COMPLETE
	};

	using EventCallback=Delegate<bool(HttpServerConnection&, const char*, size_t)>;
	using Events=ObjectMap<EventType, NodeList<EventCallback>>;

	/**
	 * @brief
	 * @param resource once set the resource will be owned by this class and destroyed when this object is destroyed
	 * @param realm
	 * @param username
	 * @param password
	 */
	HttpEventedResource(HttpResource* resource)
	{
		delegate = resource;

		onUrlComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
			auto list = getEvents(HttpEventedResource::EVENT_PATH);
			if(list != nullptr) {
				auto start = list->getHead();
				while(start != nullptr) {
					bool success = start->data(connection, nullptr, 0);
					if(!success) {
						auto request = connection.getRequest();
						request->args = (void*)1;
						break;
					}
					start = start->next;
				}
			}

			return 0;
		};

		onHeadersComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
			if(request.args) {
				// skip headers
				return 0;
			}

			auto list = getEvents(HttpEventedResource::EVENT_HEADERS);
			if(list != nullptr) {
				auto start = list->getHead();
				while(start != nullptr) {
					bool success = start->data(connection, nullptr, 0);
					if(!success) {
						auto request = connection.getRequest();
						request->args = (void*)1;
						break;
					}
					start = start->next;
				}
			}

			if(!request.args && delegate->onHeadersComplete) {
				return delegate->onHeadersComplete(connection, request, response);
			}

			return 0;
		};

		onBody = [this](HttpServerConnection& connection, HttpRequest& request, const char* at, int length) -> int {
			if(request.args) {
				// skip headers
				return 0;
			}

			auto list = getEvents(HttpEventedResource::EVENT_BODY);
			if(list != nullptr) {
				auto start = list->getHead();
				while(start != nullptr) {
					bool success = start->data(connection, at, length);
					if(!success) {
						auto request = connection.getRequest();
						request->args = (void*)1;
						break;
					}
					start = start->next;
				}
			}

			if(!request.args && delegate->onBody) {
				return delegate->onBody(connection, request, at, length);
			}

			return 0;
		};

//		onUpgrade = HttpServerConnectionUpgradeDelegate(&HttpEventedResource::forwardUpgrade, this);
		onRequestComplete = [this](HttpServerConnection& connection, HttpRequest& request, HttpResponse& response) -> int {
			if(request.args) {
				// skip headers
				return 0;
			}

			auto list = getEvents(HttpEventedResource::EVENT_COMPLETE);
			if(list != nullptr) {
				auto start = list->getHead();
				while(start != nullptr) {
					bool success = start->data(connection, nullptr, 0);
					if(!success) {
						auto request = connection.getRequest();
						request->args = (void*)1;
						break;
					}
					start = start->next;
				}
			}

			if(!request.args && delegate->onHeadersComplete) {
				return delegate->onHeadersComplete(connection, request, response);
			}

			return 0;
		};
	}


	bool addEvent(EventType type, EventCallback callback, int priority = 0)
	{
		NodeList<EventCallback>* list = events[type];
		if(list == nullptr) {
			list = new NodeList<EventCallback>();
			events[type] = list;
		}

		return list->add(callback, priority);
	}

	const NodeList<EventCallback>* getEvents(EventType type) const
	{
		return events[type];
	}

	~HttpEventedResource()
	{
		delete delegate;
	}

private:
	const HttpResource* delegate = nullptr; // << once set the delegate is owned by this class

	Events events;

//	int checkHeaders(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);
//	int forwardBody(HttpServerConnection& connection, HttpRequest& request, const char* at, int length);
//	int forwardUpgrade(HttpServerConnection& connection, HttpRequest&, char* at, int length);
//	int forwardRequestComplete(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);
};
