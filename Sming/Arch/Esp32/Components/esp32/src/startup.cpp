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
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <nvs_flash.h>
#include <esp_tasks.h>
#include <debug_progmem.h>
#include <Platform/System.h>
#include <driver/hw_timer.h>
#include <driver/uart.h>
#include <Storage.h>

#ifndef ESP32_STACK_SIZE
#define ESP32_STACK_SIZE 16384U
#endif

extern void init();

namespace
{
static constexpr uint32_t WDT_TIMEOUT_MS{8000};

void esp_init_flash()
{
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
}

void esp_init_wifi()
{
	esp_netif_init();
	esp_event_loop_create_default();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void main(void*)
{
	assert(esp_task_wdt_init(WDT_TIMEOUT_MS, true) == ESP_OK);
	assert(esp_task_wdt_add(NULL) == ESP_OK);
	assert(esp_task_wdt_status(NULL) == ESP_OK);

	hw_timer_init();

	smg_uart_detach_all();

	esp_init_flash();
	esp_init_wifi();
	ets_init_tasks();
	Storage::initialize();
	System.initialize();
	init();

	while(true) {
		esp_task_wdt_reset();
		ets_service_tasks();
	}
}

} // namespace

extern "C" void app_main(void)
{
#ifdef CONFIG_FREERTOS_UNICORE
	xTaskCreate(loop, "Sming", ESP32_STACK_SIZE, nullptr, 1, nullptr);
#else
	esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(1));
	xTaskCreatePinnedToCore(main, "Sming", ESP32_STACK_SIZE, nullptr, 1, nullptr, 1);
#endif
}
