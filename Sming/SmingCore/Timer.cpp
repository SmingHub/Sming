/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "Timer.h"

Timer& Timer::initializeMs(uint32_t milliseconds, InterruptCallback callback)
{
	setCallback(callback);
	setIntervalMs(milliseconds);
	return *this;
}

Timer& Timer::initializeUs(uint32_t microseconds, InterruptCallback callback)
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

Timer& Timer::initializeMs(uint32_t milliseconds, TimerDelegateStdFunction delegateFunction)
{
	setCallback(delegateFunction);
	setIntervalMs(milliseconds);
	return *this;
}

Timer& Timer::initializeUs(uint32_t microseconds, TimerDelegateStdFunction delegateFunction)
{
	setCallback(delegateFunction);
	setIntervalUs(microseconds);
	return *this;
}

void Timer::start(bool repeating /* = true*/)
{
	this->repeating = repeating;
	stop();
	if(interval == 0)
		return;

	simpleTimer.setCallback(
		[](void* arg) {
			auto tmr = reinterpret_cast<Timer*>(arg);
			if(tmr)
				tmr->processing();
		},
		this);

	if(interval > 10000) {
		simpleTimer.startMs(interval / 1000, longIntervalCounterLimit || repeating);
	} else {
		simpleTimer.startUs(interval, repeating);
	}

	started = true;
}

void Timer::stop()
{
	if(started) {
		simpleTimer.stop();
		started = false;
		longIntervalCounter = 0;
	}
}

void Timer::setIntervalUs(uint64_t microseconds /* = 1000000*/)
{
	if(microseconds > MAX_OS_TIMER_INTERVAL_US) {
		// interval too large, calculate a good divider
		int div = (microseconds / MAX_OS_TIMER_INTERVAL_US) + 1; // integer division, intended

		// We will lose some precision here but its so small it won't matter.
		// Error will be microseconds mod div which can at most be div-1.
		// For small long intervals (and thus div) we are talking a few us.
		// Nothing to worry about since we are going to use the millisecond timer
		// so we wont have that accuracy anyway.

		interval = microseconds / div;
		longIntervalCounter = 0;
		longIntervalCounterLimit = div;
	} else {
		interval = microseconds;
		longIntervalCounter = 0;
		longIntervalCounterLimit = 0;
	}

	if(started)
		restart();
}

void Timer::setIntervalMs(uint32_t milliseconds /* = 1000000*/)
{
	setIntervalUs(((uint64_t)milliseconds) * 1000);
}

void Timer::setCallback(InterruptCallback interrupt)
{
	ETS_INTR_LOCK();
	callback = interrupt;
	delegateFunc = nullptr;
	delegateStdFunc = nullptr;
	ETS_INTR_UNLOCK();

	if(!interrupt)
		stop();
}

void Timer::setCallback(TimerDelegate delegateFunction)
{
	ETS_INTR_LOCK();
	callback = nullptr;
	delegateFunc = delegateFunction;
	delegateStdFunc = nullptr;
	ETS_INTR_UNLOCK();

	if(!delegateFunction)
		stop();
}

void Timer::setCallback(const TimerDelegateStdFunction& delegateFunction)
{
	ETS_INTR_LOCK();
	callback = nullptr;
	delegateFunc = nullptr;
	delegateStdFunc = delegateFunction;
	ETS_INTR_UNLOCK();

	if(!delegateFunction)
		stop();
}

void Timer::processing()
{
	if(longIntervalCounterLimit > 0) {
		// we need to handle a long interval.
		longIntervalCounter++;

		if(longIntervalCounter < longIntervalCounterLimit)
			return;

		// reset counter since callback will fire.
		longIntervalCounter = 0;

		// For long intervals os_timer is set to repeating.
		// Stop timer if it was not a repeating timer.
		if(!repeating)
			stop();
	}

	tick();
}

void Timer::tick()
{
	if(callback) {
		callback();
	} else if(delegateFunc) {
		delegateFunc();
	} else if(delegateStdFunc) {
		delegateStdFunc();
	} else {
		stop();
	}
}
