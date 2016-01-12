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

Timer& Timer::initializeMs(uint32_t milliseconds, InterruptCallback callback/* = NULL*/)
{
	setCallback(callback);
	setIntervalMs(milliseconds);
	return *this;
}

Timer& Timer::initializeUs(uint32_t microseconds, InterruptCallback callback/* = NULL*/)
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
	this->repeating = repeating;
	stop();
	if(interval == 0 || (!callback && !delegate_func)) 
		return;
	
	ets_timer_setfn(&timer, (os_timer_func_t *)processing, this);	
	if (interval > 10000) 
	{
		ets_timer_arm_new(&timer, (uint32_t)(interval / 1000), 
				(long_intvl_cntr_lim > 0 ? true : repeating), 1); // msec
	}
	else 
	{
		ets_timer_arm_new(&timer, (uint32_t)interval, repeating, 0); 		  // usec
	}
	
	started = true;
}

void Timer::stop()
{
	if (!started) return;
	ets_timer_disarm(&timer);
	started = false;
	long_intvl_cntr = 0;
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

uint64_t Timer::getIntervalUs()
{
	if(long_intvl_cntr_lim > 0) {
		return interval * long_intvl_cntr_lim;
	}

	return interval;
}

uint32_t Timer::getIntervalMs()
{
	return (uint32_t)getIntervalUs() / 1000;
}

void Timer::setIntervalUs(uint64_t microseconds/* = 1000000*/)
{
	if(microseconds > MAX_OS_TIMER_INTERVAL_US)
	{
		// interval to large, calculate a good divider.
		int div = (microseconds / MAX_OS_TIMER_INTERVAL_US) + 1; // integer division, intended

		// We will lose some precision here but its so small it won't matter.
		// Error will be microseconds mod div which can at most be div-1.
		// For small long intervals (and thus div) we are talking a few us.
		// Nothing to worry about since we are going to use the millisecond timer
		// so we wont have that accuracy anyway.

		interval = microseconds / div;
		long_intvl_cntr = 0;
		long_intvl_cntr_lim = div;
	}
	else 
	{
		interval = microseconds;
		long_intvl_cntr = 0;
		long_intvl_cntr_lim = 0;
	}

	if (started)
		restart();
}

void Timer::setIntervalMs(uint32_t milliseconds/* = 1000000*/)
{
	setIntervalUs(((uint64_t)milliseconds) * 1000);
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
	else {
		if(ptimer->long_intvl_cntr_lim > 0)
		{
			// we need to handle a long interval.
			ptimer->long_intvl_cntr++;

			if(ptimer->long_intvl_cntr < ptimer->long_intvl_cntr_lim)
			{
				return;
			}
			else 
			{
				// reset counter since callback will fire.
				ptimer->long_intvl_cntr = 0;
				
				// stop timer if it was not a repeating timer.
				// for long intervals os_timer is set to repeating,
				// therefore it must be stopped.
				if(!ptimer->repeating)
					ptimer->stop();
			}
		}

		if (ptimer->callback)
		{
			ptimer->callback();
		}
		else if (ptimer->delegate_func)
		{
			ptimer->delegate_func();
		}
	}

}
