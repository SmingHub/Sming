/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * os_timer.h
 *
 * This implementation mimics the behaviour of the ESP8266 Non-OS SDK timers,
 * using Timer2 as the reference. The ESP32 uses a 1MHz reference so all times
 *
 * The ESP32 IDF contains more sophisticated timer implementations, but also
 * this same API which it refers to as the 'legacy' timer API.
 */

#pragma once

#include <cstdint>

// Disarmed
#define OS_TIMER_DEFAULT()                                                                                             \
	{                                                                                                                  \
	}

/**
 * @defgroup os_timer OS Timer API
 * @ingroup drivers
 * @{
 */

using smg_timer_func_t = void (*)(void* arg);

struct esp_timer;

struct smg_timer_t {
	struct esp_timer* handle;
	smg_timer_func_t timer_func;
	void* timer_arg;
};

/*
 * Re-map the os_timer_* definitions to avoid conflict with the real SDK implementations.
 * Those are provided as a 'legacy' API which seems to be used only by WiFi in the SDK.
 */
#define os_timer_func_t smg_timer_func_t
#define os_timer_t smg_timer_t

#define os_timer_arm smg_timer_arm
#define os_timer_arm_us smg_timer_arm_us
#define os_timer_disarm smg_timer_disarm
#define os_timer_setfn smg_timer_setfn
#define os_timer_arm_ticks smg_timer_arm_ticks
#define os_timer_expire smg_timer_expire
#define os_timer_done smg_timer_done

/**
 * @brief Set a software timer using the Timer2 tick value
 * @param ptimer Timer structure
 * @param ticks Tick count duration for the timer
 * @param repeat_flag true if timer will automatically repeat
 *
 * This function has been added to Sming for more efficient and flexible use of
 * software timers. It can be used alongside the SDK `os_timer_arm_new()` function.
 */
void smg_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag);

void smg_timer_setfn(os_timer_t* ptimer, os_timer_func_t pfunction, void* parg);
void smg_timer_arm_us(os_timer_t* ptimer, uint32_t time_us, bool repeat_flag);
void smg_timer_arm(os_timer_t* ptimer, uint32_t time_ms, bool repeat_flag);
void smg_timer_disarm(os_timer_t* ptimer);
void smg_timer_done(os_timer_t* ptimer);

static inline uint64_t smg_timer_expire(const os_timer_t* ptimer)
{
	if(ptimer == nullptr || ptimer->handle == nullptr) {
		return 0;
	}
	// First field is 'alarm': See esp_timer.c.
	return *reinterpret_cast<uint64_t*>(ptimer->handle);
}

/** @} */
