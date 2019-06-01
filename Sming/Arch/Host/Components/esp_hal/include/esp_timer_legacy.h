#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void os_timer_func_t(void* timer_arg);

struct os_timer_t {
	struct os_timer_t* timer_next;
	uint32_t timer_expire;
	uint32_t timer_period;
	os_timer_func_t* timer_func;
	void* timer_arg;
};

void os_timer_arm(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag);
void os_timer_arm_us(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag);
void os_timer_disarm(struct os_timer_t* ptimer);
void os_timer_setfn(struct os_timer_t* ptimer, os_timer_func_t* pfunction, void* parg);

// Hook function to service timers
void host_service_timers();

#ifdef __cplusplus
}
#endif
