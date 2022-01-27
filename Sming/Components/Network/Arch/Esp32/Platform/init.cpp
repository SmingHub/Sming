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
	auto eventHandler = [](void* arg, esp_event_base_t base, int32_t id, void* data) -> void {
		using namespace SmingInternal::Network;
		debugf("event %s|%d\n", base, id);
		station.eventHandler(base, id, data);
		accessPoint.eventHandler(base, id, data);
		events.eventHandler(base, id, data);
	};
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, eventHandler, nullptr));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, eventHandler, nullptr));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}
