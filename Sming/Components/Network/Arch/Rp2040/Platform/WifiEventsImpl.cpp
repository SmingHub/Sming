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

// #define WIFI_DEBUG_PROBE
// #define WIFI_DEBUG_SCAN

namespace
{
// Information Elements
void print_eid([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t datalen)
{
#ifdef ENABLE_WIFI_DEBUG
	for(unsigned i = 0; i < datalen;) {
		auto id = data[i++];
		auto len = data[i++];
		m_printHex(String(id).padLeft(3).c_str(), &data[i], len);
		i += len;
	}
#endif
}

} // namespace

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
#ifdef ENABLE_WIFI_DEBUG
	bool debug{true};

#ifndef WIFI_DEBUG_PROBE
	debug &= (info.ev.event_type != CYW43_EV_PROBREQ_MSG);
	debug &= (info.ev.event_type != CYW43_EV_P2P_PROBREQ_MSG);
#endif

#ifndef WIFI_DEBUG_SCAN
	debug &= (info.ev.event_type != CYW43_EV_ESCAN_RESULT);
#endif

	if(debug) {
		debug_i("EV.version = 0x%04x", __builtin_bswap16(info.ev.version));
		debug_i("EV.flags = %u", info.ev.flags);
		debug_i("EV.event_type = %u", info.ev.event_type);
		debug_i("EV.status = %u", info.ev.status);
		debug_i("EV.reason = %u", info.ev.reason);
		debug_i("EV.auth_type = %u", info.ev.auth_type);
		debug_i("EV.datalen = %u", info.ev.datalen);
		debug_i("EV.addr = %s", MacAddress(info.ev.addr).toString().c_str());
		debug_i("EV.ifname = %s", String(info.ev.ifname, 16).c_str());
		debug_i("EV.ifidx = %u", info.ev.ifidx);
		debug_i("EV.bsscfgidx = %u", info.ev.bsscfgidx);
		// debug_hex(INFO, "DATA", info.ev.data, info.ev.datalen);
	}
#endif

	[[maybe_unused]] unsigned off{0};
	int rssi{0};
	MacAddress addr(info.ev.addr);

	switch(info.ev.event_type) {
	case CYW43_EV_SET_SSID:
#ifdef ENABLE_WIFI_DEBUG
		debug_hex(INFO, "SSID", info.ev.data, info.ev.datalen);
#endif
		break;

	case CYW43_EV_LINK:
		if(info.ev.status == CYW43_STATUS_SUCCESS && info.ev.ifidx == CYW43_ITF_STA) {
			auto callback = [](struct netif* netif) -> void { events.stationChanged(netif); };
			netif_set_status_callback(&cyw43_state.netif[CYW43_ITF_STA], callback);
			if(onSTAConnect) {
				System.queueCallback([addr]() {
					if(events.onSTAConnect) {
						events.onSTAConnect(station.getSSID(), addr, station.getChannel());
					}
				});
			}
		}
		break;

	case CYW43_EV_DISASSOC_IND:
		if(info.ev.ifidx == CYW43_ITF_STA && onSTADisconnect) {
			String ssid = WifiStation.getSSID();
			auto reason = WifiDisconnectReason(info.ev.reason);
			System.queueCallback([ssid, addr, reason]() {
				if(events.onSTADisconnect) {
					events.onSTADisconnect(ssid, addr, reason);
				}
			});
		}
		break;

	case CYW43_EV_ASSOC_IND:
		print_eid(info.ev.data, info.ev.datalen);
		if(info.ev.ifidx == CYW43_ITF_AP && onSOFTAPConnect) {
			System.queueCallback([addr]() {
				if(events.onSOFTAPConnect) {
					unsigned aid{0};
					events.onSOFTAPConnect(addr, aid);
				}
			});
		}
		break;

	case CYW43_EV_DEAUTH_IND:
		if(info.ev.ifidx == CYW43_ITF_AP) {
			if(info.ev.reason == CYW43_REASON_DISASSOC && onSOFTAPDisconnect) {
				System.queueCallback([addr]() {
					if(events.onSOFTAPDisconnect) {
						unsigned aid{0};
						events.onSOFTAPDisconnect(addr, aid);
					}
				});
			}
		} else if(info.ev.ifidx == CYW43_ITF_STA) {
			if(onSTADisconnect) {
				String ssid = WifiStation.getSSID();
				System.queueCallback([ssid, addr]() {
					if(events.onSTADisconnect) {
						events.onSTADisconnect(ssid, addr, WIFI_DISCONNECT_REASON_AUTH_FAIL);
					}
				});
			}
		}
		break;

	case CYW43_EV_P2P_PROBREQ_MSG: {
		auto rxm = reinterpret_cast<const wl_rx_mgmt_data_t*>(info.ev.data);
		off = sizeof(*rxm);
#ifdef WIFI_DEBUG_PROBE
		debug_i("[PB] ver %u, channel 0x%04x, rssi %d, mactime 0x%08x, rate 0x%08x", __builtin_bswap16(rxm->version),
				__builtin_bswap16(rxm->channel), __builtin_bswap32(rxm->rssi), __builtin_bswap32(rxm->mactime),
				__builtin_bswap32(rxm->rate));
#endif
		rssi = rxm->rssi;
		[[fallthrough]];
	}

	case CYW43_EV_PROBREQ_MSG: {
#ifdef WIFI_DEBUG_PROBE
		auto m = reinterpret_cast<const struct hostap_ieee80211_mgmt*>(&info.ev.data[off]);
		debug_i("[PB] fc 0x%04x, dur 0x%04x, da %s, sa %s, bssid %s, seq 0x%04x", m->frame_control, m->duration,
				MacAddress(m->da).toString().c_str(), MacAddress(m->sa).toString().c_str(),
				MacAddress(m->bssid).toString().c_str(), m->seq_ctrl);
		off += 24;
		print_eid(&msg->data[off], datalen - off);
#endif
		if(onSOFTAPProbeReqRecved) {
			System.queueCallback([rssi, addr]() {
				if(events.onSOFTAPProbeReqRecved) {
					events.onSOFTAPProbeReqRecved(rssi, addr);
				}
			});
		}
		break;
	}
	}
}

} // namespace SmingInternal::Network
