/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEventsImpl.h - Esp8266 WiFi events
 *
 ****/

#pragma once

#include <Platform/WifiEvents.h>
#include <Platform/Station.h>
#include <esp_event.h>

class WifiEventsImpl : public WifiEventsClass
{
public:
	static StationConnectionStatus stationConnectionStatus;

public:
	WifiEventsImpl();

private:
	void WifiEventHandler(void* arg, esp_event_base_t base, int32_t id, void* data);
};
