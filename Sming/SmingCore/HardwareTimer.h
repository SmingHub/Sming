/*
 * HWTimer.h
 *
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 23.11.2015 by johndoe
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HWTIMER_H_
#define _SMING_CORE_HWTIMER_H_

#include "../SmingCore/Interrupts.h"
#include "../SmingCore/Delegate.h"

#define MAX_HW_TIMER_INTERVAL_US 0x7fffff
#define MIN_HW_TIMER_INTERVAL_US 0x32

typedef Delegate<void()> TimerDelegate;

class Hardware_Timer
{
public:
	Hardware_Timer();
	virtual ~Hardware_Timer();

	Hardware_Timer& IRAM_ATTR initializeUs(uint32_t microseconds, InterruptCallback callback = NULL); // Init in Microseconds.
	Hardware_Timer& IRAM_ATTR initializeMs(uint32_t milliseconds, InterruptCallback callback = NULL); // Init in Milliseconds.

	bool IRAM_ATTR start(bool repeating = true);
	bool __forceinline IRAM_ATTR startOnce() { return start(false); }
	bool IRAM_ATTR stop();
	bool IRAM_ATTR restart();
	bool isStarted();

	uint32_t getIntervalUs();
	uint32_t getIntervalMs();

    bool IRAM_ATTR setIntervalUs(uint32_t microseconds = 1000000);
    bool IRAM_ATTR setIntervalMs(uint32_t milliseconds = 1000000);

    void IRAM_ATTR setCallback(InterruptCallback callback);

    void __forceinline IRAM_ATTR call() {
    	if (callback) {
    		callback();
    	}
	}


private:
    uint32_t interval = 0;
    InterruptCallback callback = nullptr;
    bool repeating = false;
    bool started = false;
};

#endif /* _SMING_CORE_HWTIMER_H_ */
