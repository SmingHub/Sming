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

#include "HardwareTimer.h"

/*
 * eagle_soc.h defines TIMER_CLK_FREQ using a divisor of 256, but Sming calls system_timer_reinit() in user_main()
 * which changes it to 4. Fortunately, we can find out which value is in use by querying the timer2_ms_flag.
 *
 * (Found this in LWIP core)
 */
extern bool timer2_ms_flag;

// Get current timer frequency, which is variable
static __forceinline uint32_t getTimerClockFreq()
{
	return timer2_ms_flag ? (APB_CLK_FREQ / 256) : (APB_CLK_FREQ / 16);
}

/*
 * Hard-code expected defaults for time/tick conversion as computations are expensive.
 * We get about 14 minutes before overflowing.
 */

uint32_t IRAM_ATTR usToTimerTicks(uint32_t us)
{
#if APB_CLK_FREQ == 80000000
	return timer2_ms_flag ? (5 * us / 16) : (5 * us);
#else
	return uint64_t(us) * getTimerClockFreq() / 1000000;
#endif
}

uint32_t IRAM_ATTR timerTicksToUs(uint32_t ticks)
{
#if APB_CLK_FREQ == 80000000
	return (timer2_ms_flag ? (16 * ticks) : ticks) / 5;
#else
	return 1000000ULL * ticks / getTimerClockFreq();
#endif
}

#define FRC1_ENABLE_TIMER BIT7
#define FRC1_AUTO_LOAD BIT6

//TIMER PREDIVED MODE
typedef enum {
	DIVDED_BY_1 = 0,   //timer clock
	DIVDED_BY_16 = 4,  //divided by 16
	DIVDED_BY_256 = 8, //divided by 256
} TIMER_PREDIVED_MODE;

typedef enum {		  //timer interrupt mode
	TM_LEVEL_INT = 1, // level interrupt
	TM_EDGE_INT = 0,  //edge interrupt
} TIMER_INT_MODE;

// Used by ISR
static HardwareTimer* isrTimer;

static void IRAM_ATTR hw_timer_isr_cb()
{
	if(isrTimer) {
		isrTimer->call();
	}
}

HardwareTimer::HardwareTimer(HardwareTimerMode mode)
{
	assert(isrTimer == nullptr);
	isrTimer = this;
	if(mode == eHWT_Maskable) {
		ETS_FRC_TIMER1_INTR_ATTACH(ets_isr_t(hw_timer_isr_cb), nullptr);
	} else {
		ETS_FRC_TIMER1_NMI_INTR_ATTACH(hw_timer_isr_cb);
	}
}

HardwareTimer::~HardwareTimer()
{
	stop();
	isrTimer = nullptr;
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

	RTC_REG_WRITE(FRC1_CTRL_ADDRESS, DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT | (repeating ? FRC1_AUTO_LOAD : 0));

	TM1_EDGE_INT_ENABLE();
	ETS_FRC1_INTR_ENABLE();
	started = true;

	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, usToTimerTicks(interval));
	return started;
}

void HardwareTimer::stop()
{
	if(started) {
		TM1_EDGE_INT_DISABLE();
		ETS_FRC1_INTR_DISABLE();
		started = false;
	}
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
