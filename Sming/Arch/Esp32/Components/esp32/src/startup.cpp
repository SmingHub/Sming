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
	hw_timer_init();

	smg_uart_detach_all();
	esp_log_set_vprintf(m_vprintf);

	auto loop = sming_create_event_loop();

#ifndef DISABLE_WIFI
	esp_network_initialise();
#endif

	System.initialize();
	Storage::initialize();
	System.queueCallback(init);

	constexpr unsigned maxEventLoopInterval{1000 / portTICK_PERIOD_MS};
	while(true) {
#ifdef CREATE_EVENT_TASK
		vTaskDelay(100);
#else
		esp_event_loop_run(loop, maxEventLoopInterval);
#endif
	}
}

} // namespace

extern "C" void __wrap_esp_newlib_init_global_stdio(const char*)
{
}

extern void sming_create_task(TaskFunction_t);

extern "C" void app_main(void)
{
#if defined(SOC_ESP32) && !CONFIG_FREERTOS_UNICORE
	constexpr unsigned core_id{1};
#else
	constexpr unsigned core_id{0};
#endif

#ifdef DISABLE_NETWORK
#define SMING_TASK_STACK_SIZE CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE
#else
#define SMING_TASK_STACK_SIZE CONFIG_LWIP_TCPIP_TASK_STACK_SIZE
#endif
	xTaskCreatePinnedToCore(main, "Sming", SMING_TASK_STACK_SIZE, nullptr, ESP_TASKD_EVENT_PRIO, nullptr, core_id);
}
