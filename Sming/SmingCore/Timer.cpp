/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../SmingCore/Timer.h"

Timer::Timer()
{
}

Timer::~Timer()
{
	stop();
}

Timer& Timer::initializeMs(uint32_t milliseconds, TimerCallback callback/* = NULL*/)
{
	setCallback(callback);
	setIntervalMs(milliseconds);
	return *this;
}

Timer& Timer::initializeUs(uint32_t microseconds, TimerCallback callback/* = NULL*/)
{
	setCallback(callback);
	setIntervalUs(microseconds);
	return *this;
}

Timer& Timer::initializeMs(uint32_t milliseconds, TimerDelegate delegateFunction)
{
	setCallback(delegateFunction);
	setIntervalMs(milliseconds);
	return *this;
}

Timer& Timer::initializeUs(uint32_t microseconds, TimerDelegate delegateFunction)
{
	setCallback(delegateFunction);
	setIntervalUs(microseconds);
	return *this;
}

void Timer::start(bool repeating/* = true*/)
{
	stop();
	if(interval == 0 || (!callback && !delegate_func)) return;
	ets_timer_setfn(&timer, (os_timer_func_t *)processing, this);
	if (interval > 1000)
		ets_timer_arm_new(&timer, interval / 1000, repeating, 1); // msec
	else
		ets_timer_arm_new(&timer, interval, repeating, 0); 		  // usec
	started = true;
}

void Timer::stop()
{
	if (!started) return;
	ets_timer_disarm(&timer);
	started = false;
}

void Timer::restart()
{
	stop();
	start();
}

bool Timer::isStarted()
{
	return started;
}

uint32_t Timer::getIntervalUs()
{
	return (uint32_t)interval;
}

uint32_t Timer::getIntervalMs()
{
	return (uint32_t)interval / 1000;
}

void Timer::setIntervalUs(uint32_t microseconds/* = 1000000*/)
{
	interval = microseconds;
	if (started)
		restart();
}

void Timer::setIntervalMs(uint32_t milliseconds/* = 1000000*/)
{
	setIntervalUs(((uint64_t)milliseconds) * 1000);
}

void Timer::setCallback(TimerCallback interrupt/* = NULL*/)
{
	ETS_INTR_LOCK();
	callback = interrupt;
	delegate_func = nullptr;
	ETS_INTR_UNLOCK();

	if (!interrupt)
		stop();
}

void Timer::setCallback(TimerDelegate delegateFunction)
{
	ETS_INTR_LOCK();
	callback = nullptr;
	delegate_func = delegateFunction;
	ETS_INTR_UNLOCK();

	if (!delegateFunction)
		stop();
}

void Timer::processing(void *arg)
{
	Timer *ptimer = (Timer*)arg;
	if (ptimer == NULL)
	{
	   return;
	}
	else if (ptimer->callback)
	{
		ptimer->callback();
	}
	else if (ptimer->delegate_func)
	{
		ptimer->delegate_func();
	}
}
