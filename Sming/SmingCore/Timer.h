/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_Timer_H_
#define _SMING_CORE_Timer_H_

#include "../SmingCore/Interrupts.h"
#include "../Wiring/WiringFrameworkDependencies.h"

class Timer
{
public:
	Timer();
	~Timer();

	// Init in Microseconds. Method return "this" reference
	Timer& IRAM_ATTR initializeUs(uint32_t microseconds = 1000000, InterruptCallback interrupt = NULL);
	// Init in Milliseconds. Method return "this" reference
	Timer& IRAM_ATTR initializeMs(uint32_t milliseconds = 1000000, InterruptCallback interrupt = NULL);
	void IRAM_ATTR start(bool repeating = true);
	void __inline IRAM_ATTR startOnce() { start(false); }
	void IRAM_ATTR stop();
	void restart();
	bool isStarted();
	uint32_t getIntervalUs();
	uint32_t getIntervalMs();
    void setIntervalUs(uint32_t microseconds = 1000000);
    void setIntervalMs(uint32_t milliseconds = 1000000);
    void setInterrupt(InterruptCallback interrupt = NULL);

    //void resume();
    //void attachInterrupt(InterruptCallback interrupt);
    //void detachInterrupt();

protected:
    static void IRAM_ATTR processing(void *arg);

private:
    os_timer_t timer;
    uint32_t interval;
    InterruptCallback callback;
    bool started;
};

#endif /* _SMING_CORE_Timer_H_ */
