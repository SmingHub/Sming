/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEvents.cpp
 *
 */

#include <esp_wifi.h>
#include "Platform/WifiEvents.h"
#include <Data/HexString.h>
#include <IPAddress.h>

WifiEventsClass WifiEvents;

void WifiEventHandler(System_Event_t* evt);

WifiEventsClass::WifiEventsClass()
{
	wifi_set_event_handler_cb([](System_Event_t* evt) { WifiEvents.WifiEventHandler(evt); });
}

static String macToStr(const uint8_t mac[])
{
	return makeHexString(mac, 6, ':');
}

void WifiEventsClass::WifiEventHandler(System_Event_t* evt)
{
	switch(evt->event) {
	case EVENT_STAMODE_CONNECTED:
		debugf("connect to ssid %s, channel %d", evt->event_info.connected.ssid, evt->event_info.connected.channel);
		if(onSTAConnect) {
			onSTAConnect((const char*)evt->event_info.connected.ssid, evt->event_info.connected.ssid_len,
						 evt->event_info.connected.bssid, evt->event_info.connected.channel);
		}
		break;
	case EVENT_STAMODE_DISCONNECTED:
		debugf("disconnect from ssid %s, reason %d", evt->event_info.disconnected.ssid,
			   evt->event_info.disconnected.reason);
		if(onSTADisconnect) {
			onSTADisconnect((const char*)evt->event_info.disconnected.ssid, evt->event_info.disconnected.ssid_len,
							evt->event_info.disconnected.bssid, evt->event_info.disconnected.reason);
		}
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		debugf("mode: %d -> %d", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
		if(onSTAAuthModeChange) {
			onSTAAuthModeChange(evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
		}
		break;
	case EVENT_STAMODE_GOT_IP: {
		IPAddress ip(evt->event_info.got_ip.ip);
		IPAddress mask(evt->event_info.got_ip.mask);
		IPAddress gw(evt->event_info.got_ip.gw);
		debugf("ip: %s, mask: %s, gw: %s", ip.toString().c_str(), mask.toString().c_str(), gw.toString().c_str());
		if(onSTAGotIP) {
			onSTAGotIP(ip, mask, gw);
		}
		break;
	}
	case EVENT_SOFTAPMODE_STACONNECTED:
		debugf("station: %s join, AID = %d", macToStr(evt->event_info.sta_connected.mac).c_str(),
			   evt->event_info.sta_connected.aid);
		if(onSOFTAPConnect) {
			onSOFTAPConnect(evt->event_info.sta_connected.mac, evt->event_info.sta_connected.aid);
		}
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		debugf("station: %s leave, AID = %d", macToStr(evt->event_info.sta_disconnected.mac),
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
