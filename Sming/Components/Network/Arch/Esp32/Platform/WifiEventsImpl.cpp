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
#include "StationImpl.h"
#include "AccessPointImpl.h"
#include <esp_event.h>
#include <nvs_flash.h>
#include <debug_progmem.h>

WifiEventsClass& WifiEvents{SmingInternal::Network::events};

void esp_network_initialise()
{
	/*
	 * Initialise NVS which IDF WiFi uses to store configuration parameters.
	 */
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	/*
	 * Initialise default WiFi stack
	 */
	esp_netif_init();
	auto eventHandler = [](void* arg, esp_event_base_t base, int32_t id, void* data) -> void {
		SmingInternal::Network::events.eventHandler(base, id, data);
	};
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, eventHandler, nullptr));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, eventHandler, nullptr));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

namespace SmingInternal
{
namespace Network
{
WifiEventsImpl events;

ip_addr_t ip(esp_ip4_addr_t ip)
{
	ip_addr_t r = IPADDR4_INIT(ip.addr);
	return r;
}

void WifiEventsImpl::eventHandler(esp_event_base_t base, int32_t id, void* data)
{
	debugf("event %s|%d\n", base, id);

	station.eventHandler(base, id, data);
	accessPoint.eventHandler(base, id, data);

	if(base == WIFI_EVENT) {
		switch(id) {
		case WIFI_EVENT_STA_START:
			break;
		case WIFI_EVENT_STA_CONNECTED: {
			wifi_event_sta_connected_t* event = reinterpret_cast<wifi_event_sta_connected_t*>(data);
			debugf("connect to ssid %s, channel %d\n", event->ssid, event->channel);
			if(onSTAConnect) {
				String ssid(reinterpret_cast<const char*>(event->ssid), event->ssid_len);
				onSTAConnect(ssid, event->bssid, event->channel);
			}
			break;
		}
		case WIFI_EVENT_STA_DISCONNECTED: {
			wifi_event_sta_disconnected_t* event = reinterpret_cast<wifi_event_sta_disconnected_t*>(data);
			debugf("disconnect from ssid %s, reason %d\n", event->ssid, event->reason);
			if(onSTADisconnect) {
				String ssid(reinterpret_cast<const char*>(event->ssid), event->ssid_len);
				auto reason = WifiDisconnectReason(event->reason);
				onSTADisconnect(ssid, event->bssid, reason);
			}
			break;
		}
		case WIFI_EVENT_STA_AUTHMODE_CHANGE: {
			wifi_event_sta_authmode_change_t* event = reinterpret_cast<wifi_event_sta_authmode_change_t*>(data);
			auto oldMode = WifiAuthMode(event->old_mode);
			auto newMode = WifiAuthMode(event->new_mode);
			debugf("mode: %d -> %d\n", oldMode, newMode);

			if((oldMode != AUTH_OPEN) && (newMode == AUTH_OPEN)) {
				// CVE-2020-12638 workaround.
				// TODO: Remove this workaround once ESP-IDF has the proper fix.
				debugf("Potential downgrade attack. Reconnecting WiFi. See CVE-2020-12638 for more details\n");
				WifiStation.disconnect();
				WifiStation.connect();
				break;
			}

			if(onSTAAuthModeChange) {
				onSTAAuthModeChange(oldMode, newMode);
			}
			break;
		}
		case WIFI_EVENT_AP_STACONNECTED: {
			wifi_event_ap_staconnected_t* event = reinterpret_cast<wifi_event_ap_staconnected_t*>(data);
			debugf("station: " MACSTR " join, AID = %d\n", MAC2STR(event->mac), event->aid);
			if(onSOFTAPConnect) {
				onSOFTAPConnect(event->mac, event->aid);
			}
			break;
		}
		case WIFI_EVENT_AP_STADISCONNECTED: {
			wifi_event_ap_stadisconnected_t* event = reinterpret_cast<wifi_event_ap_stadisconnected_t*>(data);
			debugf("station: " MACSTR "leave, AID = %d\n", MAC2STR(event->mac), event->aid);
			if(onSOFTAPDisconnect) {
				onSOFTAPDisconnect(event->mac, event->aid);
			}
			break;
		}
		case WIFI_EVENT_AP_PROBEREQRECVED: {
			wifi_event_ap_probe_req_rx_t* event = reinterpret_cast<wifi_event_ap_probe_req_rx_t*>(data);
			if(onSOFTAPProbeReqRecved) {
				onSOFTAPProbeReqRecved(event->rssi, event->mac);
			}
			break;
		}

		default:
			break;

		} // switch id
	} else if(base == IP_EVENT) {
		switch(id) {
		case IP_EVENT_STA_GOT_IP: {
			ip_event_got_ip_t* event = reinterpret_cast<ip_event_got_ip_t*>(data);
			debugf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR "\n", IP2STR(&event->ip_info.ip),
				   IP2STR(&event->ip_info.netmask), IP2STR(&event->ip_info.gw));
			if(onSTAGotIP) {
				onSTAGotIP(ip(event->ip_info.ip), ip(event->ip_info.netmask), ip(event->ip_info.gw));
			}
			break;
		}
		case IP_EVENT_STA_LOST_IP: {
			// TODO: ESP32 station lost IP and the IP is reset to 0
			break;
		}
		case IP_EVENT_AP_STAIPASSIGNED: {
			// TODO: ESP32 soft-AP assign an IP to a connected station
			break;
		}
		default:
			break;
		} // switch id
	}
}

} // namespace Network
} // namespace SmingInternal
