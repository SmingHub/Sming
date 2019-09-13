/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * os_timer.cpp
 *
 * @author: 13 August 2018 - mikee47 <mike@sillyhouse.net>
 *
 * An alternative method for setting software timers based on the tick count.
 *
 * Technical notes
 * ---------------
 *
 * This information was obtained by examining the SDK timer function assembly code
 * from SDK versions 1.5.4, 2.2 and 3.0.
 *
 * Software timers for the ESP8266 are defined by an `os_timer_t` structure.
 * When armed, the structure is contained in a queue ordered by expiry time.
 * Thus, the first timer is the next one to expire and the expiry time is programmed
 * into the Timer2 alarm register (counter compare). The timer alarm interrupt simply
 * queues a task to handle the event.
 *
 * The timer task dequeues the timer (setting `timer_next` to -1) and invokes the
 * user-provided callback routine. If it is a repeating timer then it is re-queued.
 * The queue is implemented as a linked list so adding and removing items is very efficient
 * and requires no additional memory allocation.
 *
 * Because the Sming Clock API handles all time/tick conversions, a new `os_timer_arm_ticks()`
 * function is used which replaces the existing `ets_timer_arm_new()` function. This makes
 * timer operation more transparent, faster.
 *
 * `ets_timer_arm_new`
 * -------------------
 *
 * This is the SDK function which implements `os_timer_arm_us` and `os_timer_arm`.
 *
 * With ms_flag = false, the maximum value for `time` is 428496729us. The SDK documentation
 * for `os_timer_arm_us` states a maximum value of 0x0FFFFFFF, which is incorrect; it probably
 * applies to earlier SDK releases.
 *
 * Note: If `system_timer_reinit()` hasn't been called then calling with `ms_flag = false` will fail.
 *
 * This figure can be derived as follows, where 0x7FFFFFFF is the maximum tick range
 * (signed comparison) and 5000000 is the Timer2 frequency with /16 prescale:
 *
 * 		0x7FFFFFFF / 5000000 = 429496729.4us = 0' 7" 9.5s
 *
 * With ms_flag = true, the limit is 428496ms which agrees with the value stated in the SDK documentation.
 *
 * Timer2 frequencies for two prescaler settings are:
 * Prescale  Frequency   Period  Range (0x7FFFFFFF ticks)
 * --------  ---------   ------  -------------------------
 * - /1      80000000    12.5ns  0'  0" 26.84s
 * - /16      5000000    200ns   0'  7"  9.5s
 * - /256      312500    3.2us   1' 54" 31.95s
 *
 * @see See also `drivers/hw_timer.h`
 *
 */

#include "include/driver/os_timer.h"
#include <driver/hw_timer.h>

/*
 * This variable points to the first timer in the queue.
 * It's a global variable defined in the Espressif SDK.
 */
extern "C" os_timer_t* timer_list;

/**
 * @brief Insert a timer into the queue
 * @param ptimer The timer to insert
 * @param expire The Timer2 tick value when this timer is due
 * @note Timer is inserted into queue according to its expiry time, and _after_ any
 * existing timers with the same expiry time. If it's inserted at the head of the
 * queue (i.e. it's the new value for `timer_list`) then the Timer2 alarm register
 * is updated.
 */
static void IRAM_ATTR timer_insert(os_timer_t* ptimer, uint32_t expire)
{
	os_timer_t* t_prev = nullptr;
	auto t = timer_list;
	while(t != nullptr) {
		if(int(t->timer_expire - expire) > 0) {
			break;
		}
		t_prev = t;
		t = t->timer_next;
	}
	if(t_prev == nullptr) {
		timer_list = ptimer;
		hw_timer2_set_alarm(expire);
	} else {
		t_prev->timer_next = ptimer;
	}
	ptimer->timer_next = t;
	ptimer->timer_expire = expire;
}

void os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	os_timer_disarm(ptimer);
	ptimer->timer_period = repeat_flag ? ticks : 0;
	ets_intr_lock();
	timer_insert(ptimer, hw_timer2_read() + ticks);
	ets_intr_unlock();
}
