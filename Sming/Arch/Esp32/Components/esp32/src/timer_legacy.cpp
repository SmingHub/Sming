#include "include/esp_timer_legacy.h"

void os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	ets_timer_arm(ptimer, ticks / 5000, repeat_flag);
}
