#include "esp_init.h"

extern "C" {
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
}

void esp_init_flash()
{
	static bool initilised = false;

	if(initilised) {
		return;
	}

	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	initilised = true;
}

void esp_init_wifi()
{
	static bool initilised = false;

	if(initilised) {
		return;
	}

	esp_init_flash();

	tcpip_adapter_init();
	esp_event_loop_create_default();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	initilised = true;
}
