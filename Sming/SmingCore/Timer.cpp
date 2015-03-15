/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../SmingCore/Timer.h"

Timer::Timer()
{
	interval = 0;
	callback = NULL;
	started = false;
}

Timer::~Timer()
{
	stop();
	callback = NULL;
	started = false;
}

Timer& Timer::initializeMs(uint32_t milliseconds, InterruptCallback interrupt/* = NULL*/)
{
	interval = milliseconds * 1000;
	callback = interrupt;
	return *this;
}

Timer& Timer::initializeUs(uint32_t microseconds, InterruptCallback interrupt/* = NULL*/)
{
	interval = microseconds;
	callback = interrupt;
	return *this;
}

void Timer::start(bool repeating/*=true*/)
{
	stop();
	ets_timer_setfn(&timer, (os_timer_func_t *)processing, this);
	started = true;
	if (interval > 1000)
		ets_timer_arm_new(&timer, interval / 1000, repeating, 1); // msec
	else
		ets_timer_arm_new(&timer, interval, repeating, 0); 		  // usec
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

/*void Timer::attachInterrupt(InterruptCallback interrupt)
{
	noInterrupts();
	callback = interrupt;
	interrupts();
}

void Timer::detachInterrupt()
{
	noInterrupts();
	callback = NULL;
	interrupts();
}*/

void Timer::processing(void *arg)
{
	Timer *timer = (Timer*)arg;
	if (timer->callback != NULL)
		timer->callback();
}
