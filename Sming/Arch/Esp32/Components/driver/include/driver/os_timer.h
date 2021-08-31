/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * os_timer.h
 *
 * This implementation mimics the behaviour of the ESP8266 Non-OS SDK timers,
 * using Timer2 as the reference (which is _not_ in microseconds!)
 *
 * The ESP32 IDF contains more sophisticated timer implementations, but also
 * this same API which it refers to as the 'legacy' timer API.
 */

#pragma once

#include <rom/ets_sys.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup os_timer OS Timer API
 * @ingroup drivers
 * @{
 */

typedef ETSTimerFunc os_timer_func_t;
typedef ETSTimer os_timer_t;

#define os_timer_arm(ptimer, ms, repeat_flag) ets_timer_arm(ptimer, ms, repeat_flag)
#define os_timer_arm_us(ptimer, us, repeat_flag) ets_timer_arm_us(ptimer, us, repeat_flag)
#define os_timer_disarm(ptimer) ets_timer_disarm(ptimer)
#define os_timer_setfn(ptimer, pfunction, parg) ets_timer_setfn(ptimer, pfunction, parg)

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

/** @} */

#ifdef __cplusplus
}
#endif
