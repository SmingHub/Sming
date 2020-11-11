/*
 * This implementation mimics the behaviour of the ESP8266 Non-OS SDK timers,
 * using Timer2 as the reference (which is _not_ in microseconds!)
 *
 * The ESP32 IDF contains more sophisticated timer implementations, but also
 * this same API which it refers to as the 'legacy' timer API.
 */

#pragma once

#include <esp32/rom/ets_sys.h>

#ifdef __cplusplus
extern "C" {
#endif

#define os_timer_func_t ETSTimerFunc
#define os_timer_t ETSTimer

void os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag);

#define os_timer_arm(ptimer, ms, repeat_flag) ets_timer_arm(ptimer, ms, repeat_flag)
#define os_timer_arm_us(ptimer, us, repeat_flag) ets_timer_arm_us(ptimer, us, repeat_flag)
#define os_timer_disarm(ptimer) ets_timer_disarm(ptimer)
#define os_timer_setfn(ptimer, pfunction, parg) ets_timer_setfn(ptimer, pfunction, parg)

#ifdef __cplusplus
}
#endif
