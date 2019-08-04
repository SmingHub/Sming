/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEventsImpl.h - Host WiFi events
 *
 ****/

#pragma once

#include "Platform/WifiEvents.h"
#include "StationImpl.h"

class WifiEventsImpl : public WifiEventsClass
{
public:
	void stationConnected(const StationImpl::ApInfo& ap)
	{
		if(onSTAConnect) {
			onSTAConnect(ap.ssid, ap.bssid, ap.channel);
		}
	}

	void stationDisconnected(const StationImpl::ApInfo& ap, WifiDisconnectReason reason)
	{
		if(onSTADisconnect) {
			onSTADisconnect(ap.ssid, ap.bssid, reason);
		}
	}

	void stationGotIp(ip_addr_t ip, ip_addr_t netmask, ip_addr_t gw)
	{
		if(onSTAGotIP) {
			onSTAGotIP(ip, netmask, gw);
		}
	}
};

extern WifiEventsImpl wifiEventsImpl;
