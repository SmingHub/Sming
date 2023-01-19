/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEventsImpl.h
 *
 ****/

#pragma once

#include <Platform/WifiEvents.h>
#include <Platform/Station.h>
#include <pico/cyw43_arch.h>

namespace SmingInternal::Network
{
struct EventInfo {
	cyw43_t& self;
	const cyw43_async_event_t& ev;
};

class WifiEventsImpl : public WifiEventsClass
{
public:
	static StationConnectionStatus stationConnectionStatus;

public:
	WifiEventsImpl();

	void stationChanged(struct netif* netif);

	void eventHandler(EventInfo& info);
};

extern WifiEventsImpl events;

} // namespace SmingInternal::Network
