/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * init.cpp
 */

#include "StationImpl.h"
#include "AccessPointImpl.h"
#include "WifiEventsImpl.h"
#include <esp_event.h>
#include <nvs_flash.h>

namespace
{
void eventHandler(void*, esp_event_base_t base, int32_t id, void* data)
{
	/*
	 * This handler is called from the wifi task, but events should only be handled in the Sming task thread.
	 * We need to interpret the associated data and take a copy for the queue.
	 * Each event is then passed to an explicit handler method.
	 */

	using namespace SmingInternal::Network;
	debugf("event %s|%d\n", base, id);

	if(base == WIFI_EVENT) {
		switch(id) {
#ifdef ENABLE_WPS
		case WIFI_EVENT_STA_WPS_ER_SUCCESS:
			System.queueCallback([event = *static_cast<const wifi_event_sta_wps_er_success_t*>(data)]() {
				station.dispatchWpsErSuccess(event);
			});
			break;
		case WIFI_EVENT_STA_WPS_ER_FAILED:
			System.queueCallback([](void*) { station.dispatchStaWpsErFailed(); });
			break;
		case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
			System.queueCallback([](void*) { station.dispatchStaWpsErTimeout(); });
			break;
		case WIFI_EVENT_STA_WPS_ER_PIN:
			System.queueCallback([](void*) { station.dispatchStaWpsErPin(); });
			break;
#endif
		case WIFI_EVENT_SCAN_DONE:
			System.queueCallback(
				[event = *static_cast<const wifi_event_sta_scan_done_t*>(data)]() { station.dispatchScanDone(event); });
			break;
		case WIFI_EVENT_STA_START:
			System.queueCallback([](void*) {
				station.dispatchStaStart();
				events.dispatchStaStart();
			});
			break;
		case WIFI_EVENT_STA_CONNECTED:
			System.queueCallback([event = *static_cast<const wifi_event_sta_connected_t*>(data)]() {
				station.dispatchStaConnected(event);
				events.dispatchStaConnected(event);
			});
			break;
		case WIFI_EVENT_STA_DISCONNECTED:
			System.queueCallback([event = *static_cast<const wifi_event_sta_disconnected_t*>(data)]() {
				station.dispatchStaDisconnected(event);
				events.dispatchStaDisconnected(event);
			});
			break;
		case WIFI_EVENT_STA_AUTHMODE_CHANGE:
			System.queueCallback([event = *static_cast<const wifi_event_sta_authmode_change_t*>(data)]() {
				events.dispatchStaAuthmodeChange(event);
			});
			break;
		case WIFI_EVENT_AP_STACONNECTED:
			System.queueCallback([event = *static_cast<const wifi_event_ap_staconnected_t*>(data)]() {
				events.dispatchApStaConnected(event);
			});
			break;
		case WIFI_EVENT_AP_STADISCONNECTED:
			System.queueCallback([event = *static_cast<const wifi_event_ap_stadisconnected_t*>(data)]() {
				events.dispatchApStaDisconnected(event);
			});
			break;
		case WIFI_EVENT_AP_PROBEREQRECVED:
			System.queueCallback([event = *static_cast<const wifi_event_ap_probe_req_rx_t*>(data)]() {
				events.dispatchApProbeReqReceived(event);
			});
			break;
		default:
			break;
		}
	} else if(base == IP_EVENT) {
		switch(id) {
		case IP_EVENT_STA_GOT_IP:
			System.queueCallback([event = *static_cast<const ip_event_got_ip_t*>(data)]() {
				station.dispatchStaGotIp(event);
				events.dispatchStaGotIp(event);
			});
			break;
		case IP_EVENT_STA_LOST_IP:
			System.queueCallback([](void*) {
				station.dispatchStaLostIp();
				events.dispatchStaLostIp();
			});
			break;
		case IP_EVENT_AP_STAIPASSIGNED:
			System.queueCallback([](void*) { events.dispatchApStaIpAssigned(); });
			break;
		default:
			break;
		}
	}
}

} // namespace

// Called from startup
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
	esp_event_loop_create_default();
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, eventHandler, nullptr));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, eventHandler, nullptr));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}
