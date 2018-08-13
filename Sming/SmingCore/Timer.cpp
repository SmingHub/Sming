/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "Timer.h"

Timer& Timer::initializeMs(uint32_t milliseconds, InterruptCallback callback /* = nullptr*/)
{
	setCallback(callback);
	setIntervalMs(milliseconds);
	return *this;
}

Timer& Timer::initializeUs(uint32_t microseconds, InterruptCallback callback /* = nullptr*/)
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
	_repeating = repeating;
	stop();
	if (_interval == 0)
		return;

	_timer.setCallback(
		[](void* arg) {
			auto tmr = reinterpret_cast<Timer*>(arg);
			if (tmr)
				tmr->processing();
		},
		this);

	if (_interval > 10000)
		_timer.startMs(_interval / 1000, _longIntervalCounterLimit || repeating);
	else
		_timer.startUs(_interval, repeating);

	_started = true;
}

void Timer::stop()
{
	if (_started) {
		_timer.stop();
		_started = false;
		_longIntervalCounter = 0;
	}
}

void Timer::setIntervalUs(uint64_t microseconds /* = 1000000*/)
{
	if (microseconds > MAX_OS_TIMER_INTERVAL_US) {
		// interval too large, calculate a good divider
		int div = (microseconds / MAX_OS_TIMER_INTERVAL_US) + 1; // integer division, intended

		/*
		 * We will lose some precision here but it's so small it won't matter.
		 * Error will be (microseconds mod div) which can at most be div-1.
		 * For small long intervals (and thus div) we are talking a few us.
		 * Nothing to worry about since we are going to use the millisecond timer
		 * so we won't have that accuracy anyway.
		*/

		_interval = microseconds / div;
		_longIntervalCounter = 0;
		_longIntervalCounterLimit = div;
	}
	else {
		_interval = microseconds;
		_longIntervalCounter = 0;
		_longIntervalCounterLimit = 0;
	}

	if (_started)
		restart();
}

void Timer::setIntervalMs(uint32_t milliseconds /* = 1000000*/)
{
	setIntervalUs(((uint64_t)milliseconds) * 1000);
}

void Timer::setCallback(InterruptCallback interrupt /* = nullptr*/)
{
	ETS_INTR_LOCK();
	_callback = interrupt;
	_delegate_func = nullptr;
	_delegate_stdfunc = nullptr;
	ETS_INTR_UNLOCK();

	if (!interrupt)
		stop();
}

void Timer::setCallback(TimerDelegate delegateFunction)
{
	ETS_INTR_LOCK();
	_callback = nullptr;
	_delegate_func = delegateFunction;
	_delegate_stdfunc = nullptr;
	ETS_INTR_UNLOCK();

	if (!delegateFunction)
		stop();
}

void Timer::setCallback(const TimerDelegateStdFunction& delegateFunction)
{
	ETS_INTR_LOCK();
	_callback = nullptr;
	_delegate_func = nullptr;
	_delegate_stdfunc = delegateFunction;
	ETS_INTR_UNLOCK();

	if (!delegateFunction)
		stop();
}

void Timer::processing()
{
	if (_longIntervalCounterLimit > 0) {
		// we need to handle a long interval.
		_longIntervalCounter++;

		if (_longIntervalCounter < _longIntervalCounterLimit)
			return;

		// reset counter since callback will fire.
		_longIntervalCounter = 0;

		// For long intervals os_timer is set to repeating.
		// Stop timer if it was not a repeating timer.
		if (!_repeating)
			stop();
	}

	tick();
}

void Timer::tick()
{
	if (_callback)
		_callback();
	else if (_delegate_func)
		_delegate_func();
	else if (_delegate_stdfunc)
		_delegate_stdfunc();
	else
		stop();
}
