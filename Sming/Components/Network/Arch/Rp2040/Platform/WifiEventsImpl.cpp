/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEventsImpl.cpp
 */

#include "WifiEventsImpl.h"
#include "StationImpl.h"
#include "AccessPointImpl.h"
#include "whd.h"

#include <debug_progmem.h>

WifiEventsClass& WifiEvents{SmingInternal::Network::events};

namespace SmingInternal::Network
{
WifiEventsImpl events;

WifiEventsImpl::WifiEventsImpl()
{
}

void WifiEventsImpl::stationChanged(struct netif* netif)
{
	const ip_addr_t* ip = netif_ip_addr4(netif);
	const ip_addr_t* netmask = netif_ip_netmask4(netif);
	const ip_addr_t* gw = netif_ip_gw4(netif);

	debug_i("Got ip %s\n", ip4addr_ntoa(ip));

	if(onSTAGotIP) {
		if(ip && gw && netmask) {
			onSTAGotIP(*ip, *gw, *netmask);
		}
	}
}

/*
    flags event_type    status reason interface
    0000  ASSOC_REQ_IE  0      0      0
    0000  AUTH          0      0      0
    0000  ASSOC_RESP_IE 0      0      0
    0000  ASSOC         0      0      0
    0001  LINK          0      0      0
    0000  PSK_SUP       6      0      0
    0000  JOIN          0      0      0
    0000  SET_SSID      0      0      0
*/
void WifiEventsImpl::eventHandler(EventInfo& info)
{
	auto msg = reinterpret_cast<const whd_event_msg*>(&info.ev);
#ifdef DEBUG_WIFI_EVENT
	auto datalen = __builtin_bswap32(msg->datalen);
	debug_i("EV.version = 0x%04x", __builtin_bswap16(msg->version));
	debug_i("EV.flags = %u", msg->flags);
	debug_i("EV.event_type = %u", msg->event_type);
	debug_i("EV.status = %u", msg->status);
	debug_i("EV.reason = %u", msg->reason);
	debug_i("EV.auth_type = %u", msg->auth_type);
	debug_i("EV.datalen = %u", datalen);
	debug_i("EV.addr = %s", MacAddress(msg->addr).toString().c_str());
	debug_i("EV.ifname = %s", String(msg->ifname, 16).c_str());
	debug_i("EV.ifidx = %u", msg->ifidx);
	debug_i("EV.bsscfgidx = %u", msg->bsscfgidx);
	debug_hex(INFO, "DATA", msg->data, datalen);
#endif

	switch(info.ev.event_type) {
	case CYW43_EV_LINK:
		if(info.ev.status == CYW43_STATUS_SUCCESS && info.ev.interface == CYW43_ITF_STA) {
			auto callback = [](struct netif* netif) -> void { events.stationChanged(netif); };
			netif_set_status_callback(&cyw43_state.netif[CYW43_ITF_STA], callback);
			if(onSTAConnect) {
				onSTAConnect(station.getSSID(), msg->addr, 0); //station.getChannel());
			}
		}
		break;

	case CYW43_EV_DISASSOC_IND:
		if(info.ev.interface == CYW43_ITF_STA && onSTADisconnect) {
			onSTADisconnect(WifiStation.getSSID(), info.ev.u.scan_result.bssid, WifiDisconnectReason(info.ev.reason));
		}
		break;

	case CYW43_EV_DEAUTH_IND:
		if(info.ev.interface == CYW43_ITF_AP && msg->reason == CYW43_REASON_DISASSOC && onSOFTAPDisconnect) {
			unsigned aid{0};
			onSOFTAPDisconnect(msg->addr, aid);
		}
		break;
	}
}

} // namespace SmingInternal::Network
