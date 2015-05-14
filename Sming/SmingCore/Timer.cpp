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

Timer& Timer::initializeUs(uint32_t microseconds/* = 1000000*/, InterruptCallback callback/* = NULL*/)
{
	setCallback(callback);
	setIntervalUs(microseconds);
	return *this;
}

Timer& Timer::initializeMs(uint32_t milliseconds/* = 1000000*/, InterruptCallback callback/* = NULL*/)
{
	return initializeUs(milliseconds * 1000, callback);
}

Timer& Timer::initializeUs(uint32_t milliseconds, Delegate<void()> delegateFunction)
{
	setCallback(delegateFunction);
	setIntervalUs(milliseconds);
	return *this;
}

Timer& Timer::initializeMs(uint32_t milliseconds, Delegate<void()> delegateFunction)
{
	return initializeUs(milliseconds * 1000, delegateFunction);
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
	return interval;
}

uint32_t Timer::getIntervalMs()
{
	return interval / 1000;
}

void Timer::setIntervalUs(uint32_t microseconds/* = 1000000*/)
{
	interval = microseconds;
	if (started)
		restart();
}

void Timer::setIntervalMs(uint32_t milliseconds/* = 1000000*/)
{
	setIntervalUs(milliseconds * 1000);
}

void Timer::setCallback(InterruptCallback interrupt/* = NULL*/)
{
	ETS_INTR_LOCK();
	callback = interrupt;
	delegate_func = nullptr;
	ETS_INTR_UNLOCK();

	if (!interrupt)
		stop();
}

void Timer::setCallback(Delegate<void()> delegateFunction)
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
