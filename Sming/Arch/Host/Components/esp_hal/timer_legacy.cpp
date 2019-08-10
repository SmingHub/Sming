
#include "include/esp_system.h"
#include "include/esp_timer_legacy.h"
#include <hostlib/threads.h>

static os_timer_t* timer_head;
static CMutex mutex;

void os_timer_arm(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag)
{
	os_timer_arm_us(ptimer, time * 1000UL, repeat_flag);
}

void os_timer_arm_us(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag)
{
	os_timer_disarm(ptimer);
	ptimer->timer_next = nullptr;
	ptimer->timer_expire = system_get_time() + time;
	ptimer->timer_period = repeat_flag ? time : 0;

	// Append to list
	mutex.lock();
	if(timer_head == nullptr) {
		timer_head = ptimer;
	} else {
		auto t = timer_head;
		while(t->timer_next != nullptr) {
			t = t->timer_next;
		}
		t->timer_next = ptimer;
	}
	mutex.unlock();
}

void os_timer_disarm(struct os_timer_t* ptimer)
{
	if(ptimer == nullptr) {
		return;
	}

	mutex.lock();
	if(timer_head != nullptr) {
		// Remove timer from list
		if(ptimer == timer_head) {
			timer_head = nullptr;
		} else {
			auto t = timer_head;
			while(t->timer_next != nullptr) {
				if(t->timer_next == ptimer) {
					t->timer_next = ptimer->timer_next;
					break;
				}
				t = t->timer_next;
			}
		}
	}
	mutex.unlock();
}

void os_timer_setfn(struct os_timer_t* ptimer, os_timer_func_t* pfunction, void* parg)
{
	if(ptimer != nullptr) {
		ptimer->timer_func = pfunction;
		ptimer->timer_arg = parg;
	}
}

// Called with mutex locked
static os_timer_t* find_expired_timer()
{
	if(timer_head == nullptr) {
		return nullptr;
	}

	auto time_now = system_get_time();
	os_timer_t* t_prev = nullptr;
	for(auto t = timer_head; t != nullptr; t_prev = t, t = t->timer_next) {
		if(int(t->timer_expire - time_now) > 0) {
			continue;
		}

		// Found an expired timer
		if(t->timer_period == 0) {
			// Non-repeating timer, remove now
			if(t == timer_head) {
				timer_head = nullptr;
			} else if(t_prev != nullptr) {
				t_prev->timer_next = t->timer_next;
			}
		} else {
			t->timer_expire = time_now + t->timer_period;
		}

		return t;
	}

	return nullptr;
}

void host_service_timers()
{
	mutex.lock();
	auto t = find_expired_timer();
	mutex.unlock();

	if(t != nullptr && t->timer_func != nullptr) {
		t->timer_func(t->timer_arg);
	}
}
