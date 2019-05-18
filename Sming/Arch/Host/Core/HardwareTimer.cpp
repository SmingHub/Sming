/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HWTimer.cpp
 *
 * Created 23.11.2015 by johndoe
 *
 ****/

#include "HardwareTimer.h"
#include <esp_timer_legacy.h>

void os_timer_arm(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag);
void os_timer_arm_us(struct os_timer_t* ptimer, uint32_t time, bool repeat_flag);
void os_timer_disarm(struct os_timer_t* ptimer);
void os_timer_setfn(struct os_timer_t* ptimer, os_timer_func_t* pfunction, void* parg);

static os_timer_t timer;

uint32_t usToTimerTicks(uint32_t us)
{
	return us;
}

uint32_t timerTicksToUs(uint32_t ticks)
{
	return ticks;
}

static void hw_timer_isr_cb(void* param)
{
	if(param != nullptr) {
		static_cast<HardwareTimer*>(param)->call();
	}
}

HardwareTimer::HardwareTimer(HardwareTimerMode mode)
{
	os_timer_setfn(&timer, hw_timer_isr_cb, this);
}

HardwareTimer::~HardwareTimer()
{
	stop();
}

HardwareTimer& HardwareTimer::initializeMs(uint32_t milliseconds, InterruptCallback callback)
{
	setCallback(callback);
	setIntervalMs(milliseconds);
	return *this;
}

HardwareTimer& HardwareTimer::initializeUs(uint32_t microseconds, InterruptCallback callback)
{
	setCallback(callback);
	setIntervalUs(microseconds);
	return *this;
}

bool HardwareTimer::start(bool repeating)
{
	this->repeating = repeating;
	stop();
	if(interval == 0 || !callback) {
		return started;
	}

	started = true;

	os_timer_arm_us(&timer, interval, repeating);

	return started;
}

void HardwareTimer::stop()
{
	os_timer_disarm(&timer);
}

bool HardwareTimer::restart()
{
	stop();
	start(repeating);
	return started;
}

bool HardwareTimer::setIntervalUs(uint32_t microseconds)
{
	if(microseconds < MAX_HW_TIMER_INTERVAL_US && microseconds > MIN_HW_TIMER_INTERVAL_US) {
		interval = microseconds;
		if(started) {
			restart();
		}
	} else {
		stop();
	}
	return started;
}

void HardwareTimer::setCallback(InterruptCallback interrupt)
{
	callback = interrupt;

	if(!interrupt) {
		stop();
	}
}
