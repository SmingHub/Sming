/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Simple class for elapse timing
 *
 ****/

#ifndef __ELAPSETIMER_H
#define __ELAPSETIMER_H

#include "HardwareTimer.h"

/*
 * Microsecond elapse timer.
 */
class ElapseTimer
{
public:
	ElapseTimer()
	{
		start();
	}

	void start()
	{
		_start = NOW();
	}

	uint32_t elapsed()
	{
		return timerTicksToUs(NOW() - _start);
	}

private:
	uint32_t _start;
};

#endif // __ELAPSETIMER_H
