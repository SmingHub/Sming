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
#include <Data/PriorityList.h>
#include <Delegate.h>

class HttpEventedResource : public HttpResource
{
public:
	enum EventType { EVENT_URL, EVENT_HEADERS, EVENT_UPGRADE, EVENT_BODY, EVENT_COMPLETE };

	using EventCallback = Delegate<bool(HttpServerConnection&, char**, int*)>;
	using EventList = PriorityList<EventCallback>;
	using Events = ObjectMap<EventType, EventList>;

	/**
	 * @brief Constructor
	 * @param resource pointer to an Http resource. The resource is owned and will be deleted at the end.
	 */
	HttpEventedResource(HttpResource* resource);

	~HttpEventedResource()
	{
		delete delegate;
	}

	Type getType() override
	{
		return Type::EVENTED_RESOURCE;
	}

	int runEvent(EventType type, HttpServerConnection& connection, char** at = nullptr, int* length = nullptr);

	bool addEvent(EventType type, EventCallback callback, int priority = 0);

	const EventList* getEvents(EventType type) const
	{
		return events[type];
	}

private:
	const HttpResource* delegate = nullptr; // << once set the delegate is owned by this class
	Events events;
};
