#include <driver/os_timer.h>
#include <Platform/Clocks.h>
#include <esp_attr.h>

void IRAM_ATTR os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	uint32_t us = OsTimerClock::ticksToTime<NanoTime::Microseconds>(ticks);
	ets_timer_arm_us(ptimer, us, repeat_flag);
}
