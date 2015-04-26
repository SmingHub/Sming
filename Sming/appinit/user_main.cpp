#include <user_config.h>
#include "../SmingCore/SmingCore.h"

extern void init();

extern "C" void user_init(void)
{
	system_timer_reinit();
	uart_div_modify(UART_ID_0, UART_CLK_FREQ / 115200);
	cpp_core_initialize();
	spiffs_mount();
	System.initialize();

	init(); // User code init
}
