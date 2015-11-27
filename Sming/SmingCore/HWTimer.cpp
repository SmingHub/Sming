/*
 * HWTimer.cpp
 *
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 23.11.2015 by johndoe
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HWTimer.h"

#define US_TO_RTC_TIMER_TICKS(t)          \
    ((t) ?                                   \
     (((t) > 0x35A) ?                   \
      (((t)>>2) * ((APB_CLK_FREQ>>4)/250000) + ((t)&0x3) * ((APB_CLK_FREQ>>4)/1000000))  :    \
      (((t) *(APB_CLK_FREQ>>4)) / 1000000)) :    \
     0)

#define FRC1_ENABLE_TIMER  BIT7
#define FRC1_AUTO_LOAD  BIT6


//TIMER PREDIVED MODE
typedef enum {
    DIVDED_BY_1 = 0,		//timer clock
    DIVDED_BY_16 = 4,	//divided by 16
    DIVDED_BY_256 = 8,	//divided by 256
} TIMER_PREDIVED_MODE;

typedef enum {			//timer interrupt mode
    TM_LEVEL_INT = 1,	// level interrupt
    TM_EDGE_INT   = 0,	//edge interrupt
} TIMER_INT_MODE;


typedef enum {
    FRC1_SOURCE = 0,
    NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;


static void IRAM_ATTR hw_timer_isr_cb(void *arg)
{
	HW_Timer *ptimer = (HW_Timer*)arg;
	ptimer->call();
}

HW_Timer::HW_Timer()
{
    ETS_FRC_TIMER1_INTR_ATTACH((void*)hw_timer_isr_cb, (void *)this);
}

HW_Timer::~HW_Timer()
{
	stop();
}

HW_Timer& HW_Timer::initializeMs(uint32_t milliseconds, TimerDelegate delegateFunction)
{
	setCallback(delegateFunction);
	setIntervalMs(milliseconds);
	return *this;
}

HW_Timer& HW_Timer::initializeUs(uint32_t microseconds, TimerDelegate delegateFunction)
{
	setCallback(delegateFunction);
	setIntervalUs(microseconds);
	return *this;
}

bool HW_Timer::start(bool repeating/* = true*/)
{
	this->repeating = repeating;
	stop();
	if(interval == 0 || !delegate_func)
		return started;

	if (this->repeating == 1) {
		RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
				FRC1_AUTO_LOAD | DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
	} else {
		RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
				DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
	}

    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();
	started = true;

    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, US_TO_RTC_TIMER_TICKS(interval));
    return started;
}

bool HW_Timer::stop()
{
	if (!started) return started;
	TM1_EDGE_INT_DISABLE();
	ETS_FRC1_INTR_DISABLE();
	started = false;
	return started;
}

bool HW_Timer::restart()
{
	stop();
	start(repeating);
	return started;
}

bool HW_Timer::isStarted()
{
	return started;
}

uint32_t HW_Timer::getIntervalUs()
{
	return interval;
}

uint32_t HW_Timer::getIntervalMs()
{
	return (uint32_t)getIntervalUs() / 1000;
}

bool HW_Timer::setIntervalUs(uint32_t microseconds/* = 1000000*/)
{
	if(microseconds < MAX_HW_TIMER_INTERVAL_US && microseconds > MIN_HW_TIMER_INTERVAL_US)
	{
		interval = microseconds;
		if (started)
			restart();
	}
	else
	{
		stop();
	}
	return started;
}

bool HW_Timer::setIntervalMs(uint32_t milliseconds/* = 1000000*/)
{
	return setIntervalUs(((uint32_t)milliseconds) * 1000);
}

void HW_Timer::setCallback(TimerDelegate delegateFunction)
{
	ETS_INTR_LOCK();
	delegate_func = delegateFunction;
	ETS_INTR_UNLOCK();

	if (!delegateFunction)
		stop();
}


HW_Timer hwTimer;
