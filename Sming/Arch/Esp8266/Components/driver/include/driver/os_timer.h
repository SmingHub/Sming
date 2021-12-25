/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * os_timer.h
 *
 * @author: 13 August 2018 - mikee47 <mike@sillyhouse.net>
 *
 * An alternative method for setting software timers based on the tick count.
 *
 */

#pragma once

#include <esp_system.h>

// Disarmed
#define OS_TIMER_DEFAULT()                                                                                             \
	{                                                                                                                  \
		.timer_next = (os_timer_t*)-1,                                                                                 \
	}

// Disarmed
#define OS_TIMER_DEFAULT()                                                                                             \
	{                                                                                                                  \
		.timer_next = (os_timer_t*)-1,                                                                                 \
	}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup os_timer OS Timer API
 * @ingroup drivers
 * @{
 */

/**
 * @brief Set a software timer using the Timer2 tick value
 * @param ptimer Timer structure
 * @param ticks Tick count duration for the timer
 * @param repeat_flag true if timer will automatically repeat
 *
 * This function has been added to Sming for more efficient and flexible use of
 * software timers. It can be used alongside the SDK `os_timer_arm_new()` function.
 */
void os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag);

static inline bool os_timer_is_armed(const os_timer_t* ptimer)
{
	return ptimer != nullptr && int(ptimer->timer_next) != -1;
}

static inline uint64_t os_timer_expire(const os_timer_t* ptimer)
{
	if(ptimer == nullptr || int(ptimer->timer_next) == -1) {
		return 0;
	}
	return ptimer->timer_expire;
}

static inline void os_timer_done(os_timer_t* ptimer)
{
	ets_timer_disarm(ptimer);
}

/** @} */

#ifdef __cplusplus
}
#endif
