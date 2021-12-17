/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * startup.cpp
 *
 */

#include <esp_system.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_task.h>
#include <esp_event.h>
#include <debug_progmem.h>
#include <Platform/System.h>
#include <driver/hw_timer.h>
#include <driver/uart.h>
#include <Storage.h>

#ifndef DISABLE_NETWORK
#include <esp_netif.h>
#ifndef DISABLE_WIFI
#include <esp_wifi.h>
#include <nvs_flash.h>
#endif
#endif

extern void init();
extern esp_event_loop_handle_t sming_create_event_loop();

namespace
{
#ifndef DISABLE_WIFI
esp_event_handler_t wifiEventHandler;

/*
 * Initialise NVS which IDF WiFi uses to store configuration parameters.
 */
void esp_init_nvs()
{
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
}

/*
 * Initialise default WiFi stack
 */
void esp_init_wifi()
{
	esp_netif_init();
	if(wifiEventHandler != nullptr) {
		ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, nullptr));
		ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifiEventHandler, nullptr));
	}
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}
#endif

void main(void*)
{
	assert(esp_task_wdt_init(CONFIG_ESP_TASK_WDT_TIMEOUT_S, true) == ESP_OK);
	assert(esp_task_wdt_add(NULL) == ESP_OK);
	assert(esp_task_wdt_status(NULL) == ESP_OK);

	hw_timer_init();

	smg_uart_detach_all();
	esp_log_set_vprintf(m_vprintf);

	auto loop = sming_create_event_loop();

#ifndef DISABLE_WIFI
	esp_init_nvs();
	esp_init_wifi();
#endif

	System.initialize();
	Storage::initialize();
	init();

	constexpr unsigned maxEventLoopInterval{1000 / portTICK_PERIOD_MS};
	while(true) {
		esp_task_wdt_reset();
		esp_event_loop_run(loop, maxEventLoopInterval);
	}
}

} // namespace

/*
 * Called from WiFi event implementation constructor.
 * Cannot register directly as event queue hasn't been created yet.
 * NOTE: May only be called once.
 */
void wifi_set_event_handler_cb(esp_event_handler_t eventHandler)
{
#ifndef DISABLE_WIFI
	wifiEventHandler = eventHandler;
#endif
}

extern void sming_create_task(TaskFunction_t);

extern "C" void app_main(void)
{
#if defined(SOC_ESP32) && !CONFIG_FREERTOS_UNICORE
	constexpr unsigned core_id{1};
#else
	constexpr unsigned core_id{0};
#endif

	esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(core_id));
	xTaskCreatePinnedToCore(main, "Sming", ESP_TASKD_EVENT_STACK, nullptr, ESP_TASKD_EVENT_PRIO, nullptr, core_id);
}
