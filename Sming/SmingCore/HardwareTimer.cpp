/*
 * HWTimer.cpp
 *
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 23.11.2015 by johndoe
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HardwareTimer.h"

#define US_TO_RTC_TIMER_TICKS(t)                                                                                       \
	((t) ? (((t) > 0x35A)                                                                                              \
				? (((t) >> 2) * ((APB_CLK_FREQ >> 4) / 250000) + ((t)&0x3) * ((APB_CLK_FREQ >> 4) / 1000000))          \
				: (((t) * (APB_CLK_FREQ >> 4)) / 1000000))                                                             \
		 : 0)

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

typedef enum {
	FRC1_SOURCE = 0,
	NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;

static Hardware_Timer* g_timer;

static void IRAM_ATTR hw_timer_isr_cb()
{
	g_timer->call();
}

Hardware_Timer::Hardware_Timer()
{
	g_timer = this;
	ETS_FRC_TIMER1_NMI_INTR_ATTACH(hw_timer_isr_cb);
}

Hardware_Timer::~Hardware_Timer()
{
	stop();
}

Hardware_Timer& Hardware_Timer::initializeMs(uint32_t milliseconds, InterruptCallback callback)
{
	setCallback(callback);
	setIntervalMs(milliseconds);
	return *this;
}

Hardware_Timer& Hardware_Timer::initializeUs(uint32_t microseconds, InterruptCallback callback)
{
	setCallback(callback);
	setIntervalUs(microseconds);
	return *this;
}

bool Hardware_Timer::start(bool repeating /* = true*/)
{
	_repeating = repeating;
	stop();
	if (_interval == 0 || !_callback)
		return _started;

	RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
				  DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT | (_repeating ? FRC1_AUTO_LOAD : 0));

	TM1_EDGE_INT_ENABLE();
	ETS_FRC1_INTR_ENABLE();
	_started = true;

	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, US_TO_RTC_TIMER_TICKS(_interval));
	return _started;
}

bool Hardware_Timer::stop()
{
	if (_started) {
		TM1_EDGE_INT_DISABLE();
		ETS_FRC1_INTR_DISABLE();
		_started = false;
	}

	return _started;
}

bool Hardware_Timer::restart()
{
	stop();
	start(_repeating);
	return _started;
}

bool Hardware_Timer::setIntervalUs(uint32_t microseconds /* = 1000000*/)
{
	if (microseconds < MAX_HW_TIMER_INTERVAL_US && microseconds > MIN_HW_TIMER_INTERVAL_US) {
		_interval = microseconds;
		if (_started)
			restart();
	}
	else
		stop();

	return _started;
}

void Hardware_Timer::setCallback(InterruptCallback interrupt)
{
	ETS_INTR_LOCK();
	_callback = interrupt;
	ETS_INTR_UNLOCK();

	if (!interrupt)
		stop();
}
