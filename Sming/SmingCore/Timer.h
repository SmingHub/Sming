/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_Timer_H_
#define _SMING_CORE_Timer_H_


#include "../SmingCore/Interrupts.h"
#include "../SmingCore/Delegate.h"
#include "../Wiring/WiringFrameworkDependencies.h"


// Default timer is millisecond timer and the maximum value is 6871947ms.
// After call system_timer_reinit,the timer will change to microsecond,
// timer and the maximum value of os_timer_arm() is 429496ms.
#define MAX_OS_TIMER_INTERVAL_US 420000000  // some haedroom ;)

typedef Delegate<void()> TimerDelegate;

class Timer
{
public:
	Timer();
	~Timer();

	// It return value for Method Chaining (return "this" reference)
	// http://en.wikipedia.org/wiki/Method_chaining
	// We provide both versions: Delegate and classic c-style callback function for performance reason (for high-frequency timers)
	// Usually only Delegate needed

	Timer& IRAM_ATTR initializeMs(uint32_t milliseconds, InterruptCallback callback = NULL); // Init in Milliseconds.
	Timer& IRAM_ATTR initializeUs(uint32_t microseconds, InterruptCallback callback = NULL); // Init in Microseconds.

	Timer& IRAM_ATTR initializeMs(uint32_t milliseconds, TimerDelegate delegateFunction = NULL); // Init in Milliseconds.
	Timer& IRAM_ATTR initializeUs(uint32_t microseconds, TimerDelegate delegateFunction = NULL); // Init in Microseconds.

	void IRAM_ATTR start(bool repeating = true);
	void __forceinline IRAM_ATTR startOnce() { start(false); }
	void IRAM_ATTR stop();
	void IRAM_ATTR restart();
	bool isStarted();

	uint64_t getIntervalUs();
	uint32_t getIntervalMs();

    void IRAM_ATTR setIntervalUs(uint64_t microseconds = 1000000);
    void IRAM_ATTR setIntervalMs(uint32_t milliseconds = 1000000);

    void IRAM_ATTR setCallback(InterruptCallback interrupt = NULL);
    void IRAM_ATTR setCallback(TimerDelegate delegateFunction);

protected:
    static void IRAM_ATTR processing(void *arg);


private:
    os_timer_t timer;
    uint64_t interval = 0;
    InterruptCallback callback = nullptr;
    TimerDelegate delegate_func = nullptr;
    bool started = false;


    // Because of the limitation in Espressif SDK a workargound
    // was added to allow for longer timer intervals.
    uint16_t long_intvl_cntr = 0;
    uint16_t long_intvl_cntr_lim = 0;
};

#endif /* _SMING_CORE_Timer_H_ */
