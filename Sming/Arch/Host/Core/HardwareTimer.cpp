/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HWTimer.cpp
 *
 * Created 23.11.2015 by johndoe
 *
 ****/

#include <hostlib/threads.h>
#include <sys/time.h>
#include "HardwareTimer.h"
#include <esp_timer_legacy.h>
#include <Services/Profiling/ElapseTimer.h>

static HardwareTimerMode hw_timer_mode;

/*
 * Simulate interrupts using a secondary thread
 */
class CTimerThread : public CThread
{
public:
	CTimerThread(HardwareTimer& timer, bool repeating)
		: CThread("hwtimer", hw_timer_mode == eHWT_Maskable ? 1 : 3), timer(timer), repeating(repeating)
	{
	}

	~CTimerThread()
	{
		sem.post();
	}

protected:
	void* thread_routine() override;

private:
	CSemaphore sem;
	HardwareTimer& timer;
	bool repeating;
};

static CTimerThread* thread;

void* CTimerThread::thread_routine()
{
#ifdef __WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif

	do {
		auto interval = timer.getIntervalUs();
		ElapseTimer elapseTimer(interval);
		/*
		 * Typical scheduler minimum timeslice
		 * @todo Determine this value by asking OS
		 */
		const unsigned SCHED_MIN = 1500;
		if(interval > SCHED_MIN) {
			timeval tv;
			gettimeofday(&tv, nullptr);
			tv.tv_usec += interval - SCHED_MIN;
			timespec to;
			to.tv_sec = tv.tv_sec + tv.tv_usec / 1000000;
			to.tv_nsec = (tv.tv_usec % 1000000) * 1000;
			if(sem.timedwait(&to)) {
				break; // cancelled
			}
			if(errno != ETIMEDOUT) {
				hostmsg("Warning! Timer thread errno = %u", errno);
				break;
			}
		}
		/*
		 * Can't guarantee we won't get pre-empted here but if we've
		 * just come out of a wait state should be fine.
		 */
		while(!elapseTimer.expired()) {
			if(sem.trywait()) {
				return nullptr; // cancelled
			}
		}
		interrupt_begin();
		timer.call();
		interrupt_end();
	} while(repeating);
	return nullptr;
}

HardwareTimer::HardwareTimer(HardwareTimerMode mode)
{
	hw_timer_mode = mode;
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

	thread = new CTimerThread(*this, repeating);
	thread->execute();
	started = true;

	return started;
}

void HardwareTimer::stop()
{
	delete thread;
	thread = nullptr;
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
