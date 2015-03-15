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

	// Init in Milliseconds. Method return "this" reference
	Timer& initializeMs(uint32_t milliseconds=1000000, InterruptCallback interrupt = NULL);
	// Init in Microseconds. Method return "this" reference
	Timer& initializeUs(uint32_t microseconds=1000000, InterruptCallback interrupt = NULL);
	void start(bool repeating=true);
	void __inline startOnce() { start(false); }
	void stop();
	void restart();
	bool isStarted();
	uint32_t getIntervalUs();
	uint32_t getIntervalMs();
	//void resume();

    //void attachInterrupt(InterruptCallback interrupt);
    //void detachInterrupt();
    //void setPeriod(long microseconds);

protected:
    static void IRAM_ATTR processing(void *arg);

private:
    os_timer_t timer;
    uint32_t interval;
    InterruptCallback callback;
    bool started;
};

#endif /* _SMING_CORE_Timer_H_ */
