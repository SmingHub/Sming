#include <driver/os_timer.h>
#include <esp_attr.h>

void IRAM_ATTR os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	ets_timer_arm(ptimer, ticks / 5000, repeat_flag);
}
