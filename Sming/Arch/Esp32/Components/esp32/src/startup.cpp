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
#include <esp_event.h>
#include <debug_progmem.h>
#include <Platform/System.h>
#include <driver/hw_timer.h>
#include <driver/uart.h>
#include <Storage.h>

extern void init();
extern void esp_network_initialise();
extern void start_sming_task_loop();

extern "C" void __wrap_esp_newlib_init_global_stdio(const char*)
{
}

extern "C" void app_main(void)
{
	hw_timer_init();

	smg_uart_detach_all();
	esp_log_set_vprintf(m_vprintf);

	esp_event_loop_create_default();

#ifndef DISABLE_WIFI
	esp_network_initialise();
#endif

	System.initialize();
	Storage::initialize();

	// Application gets called outside main thread at startup
	// Things like smartconfig won't work if called via task queue
	init();

	start_sming_task_loop();
}
