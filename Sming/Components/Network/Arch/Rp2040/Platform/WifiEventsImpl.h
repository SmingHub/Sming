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
struct whd_event_msg {
	uint16_t version;
	uint16_t flags;		 /* see flags below */
	uint32_t event_type; /* Message (see below) */
	uint32_t status;	 /* Status code (see below) */
	uint32_t reason;	 /* Reason code (if applicable) */
	uint32_t auth_type;  /* WLC_E_AUTH */
	uint32_t datalen;	/* data buf */
	uint8_t addr[6];	 /* Station address (if applicable) */
	char ifname[16];	 /* name of the packet incoming interface */
	uint8_t ifidx;		 /* destination OS i/f index */
	uint8_t bsscfgidx;   /* source bsscfg index */
	uint8_t data[];
};
struct EventInfo {
	cyw43_t& self;
	const struct whd_event_msg& ev;
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
