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

extern void init();
extern esp_event_loop_handle_t sming_create_event_loop();
extern void esp_network_initialise();

namespace
{
void main(void*)
{
	int err;
	(void)err;
#if ESP_IDF_VERSION_MAJOR < 5
	err = esp_task_wdt_init(CONFIG_ESP_TASK_WDT_TIMEOUT_S, true);
#else
	esp_task_wdt_config_t twdt_config{
		.timeout_ms = 8000,
		.trigger_panic = true,
	};
	err = esp_task_wdt_init(&twdt_config);
#endif
	assert(err == ESP_OK);

	err = esp_task_wdt_add(nullptr);
	assert(err == ESP_OK);

	hw_timer_init();

	smg_uart_detach_all();
	esp_log_set_vprintf(m_vprintf);

	auto loop = sming_create_event_loop();

#ifndef DISABLE_WIFI
	esp_network_initialise();
#endif

	System.initialize();
	Storage::initialize();
	init();

	constexpr unsigned maxEventLoopInterval{1000 / portTICK_PERIOD_MS};
	while(true) {
		esp_task_wdt_reset();
#ifdef CREATE_EVENT_TASK
		vTaskDelay(100);
#else
		esp_event_loop_run(loop, maxEventLoopInterval);
#endif
	}
}

} // namespace

extern void sming_create_task(TaskFunction_t);

extern "C" void app_main(void)
{
#if defined(SOC_ESP32) && !CONFIG_FREERTOS_UNICORE
	constexpr unsigned core_id{1};
#else
	constexpr unsigned core_id{0};
#endif

#if ESP_IDF_VERSION_MAJOR < 5
	esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(core_id));
#endif
	xTaskCreatePinnedToCore(main, "Sming", ESP_TASKD_EVENT_STACK, nullptr, ESP_TASKD_EVENT_PRIO, nullptr, core_id);
}
