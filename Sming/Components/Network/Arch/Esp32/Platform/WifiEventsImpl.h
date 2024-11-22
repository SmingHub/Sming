/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEventsImpl.h - Esp32 WiFi events
 *
 ****/

#pragma once

#include <Platform/WifiEvents.h>
#include <Platform/Station.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <debug_progmem.h>

namespace SmingInternal::Network
{
class WifiEventsImpl : public WifiEventsClass
{
public:
	void dispatchStaStart()
	{
	}

	void dispatchStaConnected(const wifi_event_sta_connected_t& event)
	{
		debugf("connect to ssid %s, channel %d\n", event.ssid, event.channel);
		if(onSTAConnect) {
			String ssid(reinterpret_cast<const char*>(event.ssid), event.ssid_len);
			onSTAConnect(ssid, event.bssid, event.channel);
		}
	}

	void dispatchStaDisconnected(const wifi_event_sta_disconnected_t& event)
	{
		debugf("disconnect from ssid %s, reason %d\n", event.ssid, event.reason);
		if(onSTADisconnect) {
			String ssid(reinterpret_cast<const char*>(event.ssid), event.ssid_len);
			auto reason = WifiDisconnectReason(event.reason);
			onSTADisconnect(ssid, event.bssid, reason);
		}
	}

	void dispatchStaAuthmodeChange(const wifi_event_sta_authmode_change_t& event)
	{
		auto oldMode = WifiAuthMode(event.old_mode);
		auto newMode = WifiAuthMode(event.new_mode);
		debugf("mode: %d -> %d\n", oldMode, newMode);

		if(onSTAAuthModeChange) {
			onSTAAuthModeChange(oldMode, newMode);
		}
	}

	void dispatchApStaConnected(const wifi_event_ap_staconnected_t& event)
	{
		debugf("station: " MACSTR " join, AID = %d\n", MAC2STR(event.mac), event.aid);
		if(onSOFTAPConnect) {
			onSOFTAPConnect(event.mac, event.aid);
		}
	}

	void dispatchApStaDisconnected(const wifi_event_ap_stadisconnected_t& event)
	{
		debugf("station: " MACSTR "leave, AID = %d\n", MAC2STR(event.mac), event.aid);
		if(onSOFTAPDisconnect) {
			onSOFTAPDisconnect(event.mac, event.aid);
		}
	}

	void dispatchApProbeReqReceived(const wifi_event_ap_probe_req_rx_t& event)
	{
		if(onSOFTAPProbeReqRecved) {
			onSOFTAPProbeReqRecved(event.rssi, event.mac);
		}
	}

	void dispatchStaGotIp(const ip_event_got_ip_t& event)
	{
		debugf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR "\n", IP2STR(&event.ip_info.ip), IP2STR(&event.ip_info.netmask),
			   IP2STR(&event.ip_info.gw));
		if(onSTAGotIP) {
			auto ip = [](esp_ip4_addr_t ip) -> IpAddress {
				ip_addr_t r = IPADDR4_INIT(ip.addr);
				return IpAddress{r};
			};
			onSTAGotIP(ip(event.ip_info.ip), ip(event.ip_info.netmask), ip(event.ip_info.gw));
		}
	}

	void dispatchStaLostIp()
	{
		// TODO: ESP32 station lost IP and the IP is reset to 0
	}

	void dispatchApStaIpAssigned()
	{
		// TODO: ESP32 soft-AP assign an IP to a connected station
	}
};

extern WifiEventsImpl events;

} // namespace SmingInternal::Network
