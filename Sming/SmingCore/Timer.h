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

//Define Callback and Delegate type void, no parameters
DELEGATE_CALLBACK (Timer,void)

class Timer
{
public:
	Timer();
	~Timer();

	// It return value for Method Chaining (return "this" reference)
	// http://en.wikipedia.org/wiki/Method_chaining

	Timer& IRAM_ATTR initializeMs(uint32_t milliseconds, TimerCallback callback = NULL); // Init in Milliseconds.
	Timer& IRAM_ATTR initializeUs(uint32_t microseconds, TimerCallback callback = NULL); // Init in Microseconds.

	Timer& IRAM_ATTR initializeMs(uint32_t milliseconds, TimerDelegate delegateFunction = NULL); // Init in Milliseconds.
	Timer& IRAM_ATTR initializeUs(uint32_t microseconds, TimerDelegate delegateFunction = NULL); // Init in Microseconds.

	void IRAM_ATTR start(bool repeating = true);
	void __forceinline IRAM_ATTR startOnce() { start(false); }
	void IRAM_ATTR stop();
	void IRAM_ATTR restart();
	bool isStarted();

	uint32_t getIntervalUs();
	uint32_t getIntervalMs();

    void IRAM_ATTR setIntervalUs(uint32_t microseconds = 1000000);
    void IRAM_ATTR setIntervalMs(uint32_t milliseconds = 1000000);

    void IRAM_ATTR setCallback(TimerCallback interrupt = NULL);
    void IRAM_ATTR setCallback(TimerDelegate delegateFunction);

protected:
    static void IRAM_ATTR processing(void *arg);


private:
    os_timer_t timer STORE_ATTR;
    uint64_t interval = 0;
    TimerCallback callback = nullptr;
    TimerDelegate delegate_func = nullptr;
    bool started = false;
};

#endif /* _SMING_CORE_Timer_H_ */
