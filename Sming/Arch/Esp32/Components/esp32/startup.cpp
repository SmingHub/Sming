/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * startup.cpp
 *
 */

#include <Platform/System.h>
#include <esp_init.h>
#include <esp_timer.h>

extern void init();

extern "C" void app_main(void)
{
//	esp_timer_dump(stdout);

	host_init_tasks();

	System.initialize();

#ifndef DISABLE_WIFI
	esp_init_wifi();
#endif

	init();

	// TODO: Call all System.onReady event listeners
	host_service_tasks();

//	esp_timer_dump(stdout);
}
