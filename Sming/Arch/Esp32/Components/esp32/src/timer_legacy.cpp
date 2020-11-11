#include "include/esp_system.h"
#include "include/esp_timer_legacy.h"
#include <driver/hw_timer.h>

void os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	ets_timer_arm((ETSTimer*)ptimer, ticks / 5000, repeat_flag);
}
