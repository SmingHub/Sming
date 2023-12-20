#include <driver/os_timer.h>
#include <driver/hw_timer.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>
#include <hardware/sync.h>
#include <muldiv.h>

#ifdef ENABLE_OSTIMER_DEBUG
#define debug_tmr(fmt, ...) m_printf("%u [TMR] " fmt "\r\n", hw_timer2_read(), ##__VA_ARGS__)

#else
#define debug_tmr(fmt, ...)                                                                                            \
	do {                                                                                                               \
	} while(0)
#endif

namespace
{
os_timer_t* timer_list;

/**
 * @brief Protect certain code blocks which are callable from interrupt context
 */
class CriticalLock
{
public:
	__forceinline CriticalLock()
	{
		level = save_and_disable_interrupts();
	}

	__forceinline ~CriticalLock()
	{
		restore_interrupts(level);
	}

private:
	uint32_t level;
};

void IRAM_ATTR timer_insert(uint32_t expire, os_timer_t* ptimer)
{
	debug_tmr("insert %p %u", ptimer, expire);

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

#ifdef ENABLE_OSTIMER_DEBUG
	t = timer_list;
	while(t != nullptr) {
		debug_tmr("%p @ %u", t, t->timer_expire);
		t = t->timer_next;
	}
#endif
}

// Schedule next timer, if there is one
void IRAM_ATTR timer_schedule()
{
	// Schedule next timer, if there is one
	if(timer_list == nullptr) {
		debug_tmr("cancel");
		// Cancel hardware timer
		timer_hw->armed = BIT(1);
		return;
	}

	constexpr int TIMER2_MIN_US{50};
	auto now = hw_timer2_read();
	if(int(timer_list->timer_expire - now) < TIMER2_MIN_US) {
		timer_hw->alarm[1] = now + TIMER2_MIN_US;
	} else {
		timer_hw->alarm[1] = timer_list->timer_expire;
	}
}

os_timer_t* find_expired_timer()
{
	if(timer_list == nullptr) {
		return nullptr;
	}

	// Using Timer2 hardware to schedule software timers
	if(timer_hw->armed & BIT(1)) {
		return nullptr;
	}

	CriticalLock lock;

	// Pop timer from head of queue
	auto t = timer_list;
	timer_list = t->timer_next;
	t->timer_next = reinterpret_cast<os_timer_t*>(-1);

	debug_tmr("fired %p", t);

	// Repeating timer?
	if(t->timer_period != 0) {
		timer_insert(t->timer_expire + t->timer_period, t);
	}

	timer_schedule();

	return t;
}

} // namespace

void IRAM_ATTR os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	if(ptimer == nullptr) {
		return;
	}

	os_timer_disarm(ptimer);
	ptimer->timer_period = repeat_flag ? ticks : 0;

	CriticalLock lock;
	timer_insert(hw_timer2_read() + ticks, ptimer);
	timer_schedule();
}

void IRAM_ATTR os_timer_arm(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag)
{
	using R = std::ratio<HW_TIMER2_CLK, 1000>;
	auto ticks = muldiv<R::num, R::den>(time);
	os_timer_arm_ticks(ptimer, ticks, repeat_flag);
}

void IRAM_ATTR os_timer_arm_us(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag)
{
	using R = std::ratio<HW_TIMER2_CLK, 1000000>;
	auto ticks = muldiv<R::num, R::den>(time);
	os_timer_arm_ticks(ptimer, ticks, repeat_flag);
}

void IRAM_ATTR os_timer_disarm(struct os_timer_t* ptimer)
{
	if(ptimer == nullptr || timer_list == nullptr || int(ptimer->timer_next) == -1) {
		return; // not armed
	}

	CriticalLock lock;

	// Remove timer from list
	if(ptimer == timer_list) {
		timer_list = ptimer->timer_next;
		timer_schedule();
	} else {
		for(auto t = timer_list; t->timer_next != nullptr; t = t->timer_next) {
			if(t->timer_next == ptimer) {
				t->timer_next = ptimer->timer_next;
				break;
			}
		}
	}
	ptimer->timer_next = reinterpret_cast<os_timer_t*>(-1);
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

void system_init_timers()
{
	// hardware_alarm_claim(1);
}

void system_service_timers()
{
	auto t = find_expired_timer();
	if(t != nullptr && t->timer_func != nullptr) {
		t->timer_func(t->timer_arg);
	}
}
