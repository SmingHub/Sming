#include <user_config.h>
#include "../SmingCore/SmingCore.h"

extern void init();

extern "C" void user_init(void)
{
	system_timer_reinit();
	cpp_core_initialize();
	spiffs_mount();
	System.initialize();

	init();
}
