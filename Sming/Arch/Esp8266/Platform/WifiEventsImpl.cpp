/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEventsImpl.cpp
 *
 *  Created on: 19 февр. 2016 г.
 *      Author: shurik
 */

#include "WifiEventsImpl.h"
#include <esp_wifi.h>

static WifiEventsImpl events;
WifiEventsClass& WifiEvents = events;

WifiEventsImpl::WifiEventsImpl()
{
	wifi_set_event_handler_cb([](System_Event_t* evt) { events.WifiEventHandler(evt); });
}

void WifiEventsImpl::WifiEventHandler(System_Event_t* evt)
{
	//	debugf("event %x\n", evt->event);

	switch(evt->event) {
	case EVENT_STAMODE_CONNECTED:
		debugf("connect to ssid %s, channel %d\n", evt->event_info.connected.ssid, evt->event_info.connected.channel);
		if(onSTAConnect) {
			String ssid(reinterpret_cast<const char*>(evt->event_info.connected.ssid),
						evt->event_info.connected.ssid_len);
			onSTAConnect(ssid, evt->event_info.connected.bssid, evt->event_info.connected.channel);
		}
		break;
	case EVENT_STAMODE_DISCONNECTED:
		debugf("disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid,
			   evt->event_info.disconnected.reason);
		if(onSTADisconnect) {
			String ssid(reinterpret_cast<const char*>(evt->event_info.disconnected.ssid),
						evt->event_info.disconnected.ssid_len);
			auto reason = WifiDisconnectReason(evt->event_info.disconnected.reason);
			onSTADisconnect(ssid, evt->event_info.disconnected.bssid, reason);
		}
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE: {
		auto oldMode = WifiAuthMode(evt->event_info.auth_change.old_mode);
		auto newMode = WifiAuthMode(evt->event_info.auth_change.new_mode);
		debugf("mode: %d -> %d\n", oldMode, newMode);
		if(onSTAAuthModeChange) {
			onSTAAuthModeChange(oldMode, newMode);
		}
		break;
	}
	case EVENT_STAMODE_GOT_IP:
		debugf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR "\n", IP2STR(&evt->event_info.got_ip.ip),
			   IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
		if(onSTAGotIP) {
			onSTAGotIP(evt->event_info.got_ip.ip, evt->event_info.got_ip.mask, evt->event_info.got_ip.gw);
		}
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		debugf("station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac),
			   evt->event_info.sta_connected.aid);
		if(onSOFTAPConnect) {
			onSOFTAPConnect(evt->event_info.sta_connected.mac, evt->event_info.sta_connected.aid);
		}
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		debugf("station: " MACSTR "leave, AID = %d\n", MAC2STR(evt->event_info.sta_disconnected.mac),
			   evt->event_info.sta_disconnected.aid);
		if(onSOFTAPDisconnect) {
			onSOFTAPDisconnect(evt->event_info.sta_disconnected.mac, evt->event_info.sta_disconnected.aid);
		}
		break;
	case EVENT_SOFTAPMODE_PROBEREQRECVED:
		if(onSOFTAPProbeReqRecved) {
			onSOFTAPProbeReqRecved(evt->event_info.ap_probereqrecved.rssi, evt->event_info.ap_probereqrecved.mac);
		}
		break;
	default:
		break;
	}
}
