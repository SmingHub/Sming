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

class WifiEventsImpl : public WifiEventsClass
{
public:
	WifiEventsImpl();

private:
	void WifiEventHandler(System_Event_t* evt);
};
