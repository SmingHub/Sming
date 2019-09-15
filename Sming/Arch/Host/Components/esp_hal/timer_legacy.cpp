#include "include/esp_system.h"
#include "include/esp_timer_legacy.h"
#include <hostlib/threads.h>
#include <driver/hw_timer.h>
#include <muldiv.h>
#include <assert.h>

static os_timer_t* timer_head;
static CMutex mutex;

static void timer_insert(uint32_t expire, os_timer_t* ptimer)
{
	os_timer_t* t_prev = nullptr;
	auto t = timer_head;
	while(t != nullptr) {
		if(int(t->timer_expire - expire) > 0) {
			break;
		}
		t_prev = t;
		t = t->timer_next;
	}
	if(t_prev == nullptr) {
		timer_head = ptimer;
	} else {
		t_prev->timer_next = ptimer;
	}
	ptimer->timer_next = t;
	ptimer->timer_expire = expire;
}

void os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	assert(ptimer != nullptr);
	//	assert(time <= MAX_OS_TIMER_INTERVAL_US);

	os_timer_disarm(ptimer);
	ptimer->timer_period = repeat_flag ? ticks : 0;
	mutex.lock();
	timer_insert(hw_timer2_read() + ticks, ptimer);
	mutex.unlock();
}

void os_timer_arm(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag)
{
	using R = std::ratio<HW_TIMER2_CLK, 1000>;
	auto ticks = muldiv<R::num, R::den>(time);
	os_timer_arm_ticks(ptimer, ticks, repeat_flag);
}

void os_timer_arm_us(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag)
{
	using R = std::ratio<HW_TIMER2_CLK, 1000000>;
	auto ticks = muldiv<R::num, R::den>(time);
	os_timer_arm_ticks(ptimer, ticks, repeat_flag);
}

void os_timer_disarm(struct os_timer_t* ptimer)
{
	assert(ptimer != nullptr);

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
		ptimer->timer_next = reinterpret_cast<os_timer_t*>(-1);
	}
	mutex.unlock();
}

void os_timer_setfn(struct os_timer_t* ptimer, os_timer_func_t* pfunction, void* parg)
{
	if(ptimer != nullptr) {
		ptimer->timer_func = pfunction;
		ptimer->timer_arg = parg;
		ptimer->timer_next = reinterpret_cast<os_timer_t*>(-1);
	}
}

// Called with mutex locked
static os_timer_t* find_expired_timer()
{
	if(timer_head == nullptr) {
		return nullptr;
	}

	auto ticks_now = hw_timer2_read();
	os_timer_t* t_prev = nullptr;
	for(auto t = timer_head; t != nullptr; t_prev = t, t = t->timer_next) {
		if(int(t->timer_expire - ticks_now) > 0) {
			// No timers due
			break;
		}

		// Found an expired timer, so remove from queue
		if(t == timer_head) {
			timer_head = t->timer_next;
		} else if(t_prev != nullptr) {
			t_prev->timer_next = t->timer_next;
		}

		// Repeating timer?
		if(t->timer_period != 0) {
			timer_insert(t->timer_expire + t->timer_period, t);
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
