#include <driver/os_timer.h>
#include <hostlib/threads.h>
#include <driver/hw_timer.h>
#include <muldiv.h>
#include <cassert>

namespace
{
os_timer_t* timer_list;
CMutex mutex;

// Called with mutex locked
void timer_insert(uint32_t expire, os_timer_t* ptimer)
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
	} else {
		t_prev->timer_next = ptimer;
	}
	ptimer->timer_next = t;
	ptimer->timer_expire = expire;
}

} // namespace

void os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	assert(ptimer != nullptr);
	//	assert(time <= MAX_OS_TIMER_INTERVAL_US);

	os_timer_disarm(ptimer);
	ptimer->timer_period = repeat_flag ? ticks : 0;
	mutex.lock();
	timer_insert(hw_timer2_read() + ticks, ptimer);
	mutex.unlock();

	// Kick main thread (which services timers) if we're due next
	if(timer_list == ptimer) {
		host_thread_kick();
	}
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

	if(intptr_t(ptimer->timer_next) == -1) {
		return;
	}

	mutex.lock();
	if(timer_list != nullptr) {
		// Remove timer from list
		if(ptimer == timer_list) {
			timer_list = ptimer->timer_next;
		} else {
			auto t = timer_list;
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
	if(ptimer == nullptr) {
		return;
	}
	os_timer_disarm(ptimer);
	ptimer->timer_func = pfunction;
	ptimer->timer_arg = parg;
}

void os_timer_done(struct os_timer_t* ptimer)
{
	os_timer_disarm(ptimer);
}

int host_service_timers()
{
	if(timer_list == nullptr) {
		return -1;
	}

	auto ticks_now = hw_timer2_read();
	auto t = timer_list;
	int ticks = t->timer_expire - ticks_now;
	if(ticks > 0) {
		// Return milliseconds until timer due
		using R = std::ratio<1000, HW_TIMER2_CLK>;
		return muldiv<R::num, R::den>(unsigned(ticks));
	}

	mutex.lock();
	// Pop timer from queue
	timer_list = t->timer_next;
	t->timer_next = reinterpret_cast<os_timer_t*>(-1);
	// Repeating timer?
	if(t->timer_period != 0) {
		timer_insert(t->timer_expire + t->timer_period, t);
	}
	mutex.unlock();

	if(t->timer_func != nullptr) {
		t->timer_func(t->timer_arg);
	}

	// Call again soon as poss.
	return 0;
}
