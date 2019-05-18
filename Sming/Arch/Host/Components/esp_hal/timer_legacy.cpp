
#include "include/esp_system.h"
#include "include/esp_timer_legacy.h"

// Dummy timer, never gets used just makes code simpler
static os_timer_t timer_head = {0};

void os_timer_arm(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag)
{
	os_timer_arm_us(ptimer, time * 1000UL, repeat_flag);
}

void os_timer_arm_us(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag)
{
	os_timer_disarm(ptimer);
	auto t = &timer_head;
	while(t->timer_next != nullptr) {
		t = t->timer_next;
	}
	ptimer->timer_next = nullptr;
	ptimer->timer_expire = system_get_time() + time;
	ptimer->timer_period = repeat_flag ? time : 0;
	t->timer_next = ptimer;
}

void os_timer_disarm(struct os_timer_t* ptimer)
{
	auto t_prev = &timer_head;
	for(auto t = t_prev->timer_next; t != nullptr; t_prev = t, t = t->timer_next) {
		if(t == ptimer) {
			t_prev->timer_next = t->timer_next;
			break;
		}
	}
}

void os_timer_setfn(struct os_timer_t* ptimer, os_timer_func_t* pfunction, void* parg)
{
	if(ptimer != nullptr) {
		ptimer->timer_func = pfunction;
		ptimer->timer_arg = parg;
	}
}

void host_service_timers()
{
	auto time_now = system_get_time();
	auto t_prev = &timer_head;
	for(auto t = t_prev->timer_next; t != nullptr; t_prev = t, t = t->timer_next) {
		if(t->timer_expire > time_now) {
			continue;
		}
		if(t->timer_period == 0) {
			// Non-repeating timer, remove now
			t_prev->timer_next = t->timer_next;
		}
		if(t->timer_func != nullptr) {
			t->timer_func(t->timer_arg);
		}
		t->timer_expire = time_now + t->timer_period;
	}
}
