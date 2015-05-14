#include "SystemClock.h"

uint32_t SystemClockClass::now()
{
	// calculate number of seconds passed since last call to now()
	while (millis() - prevMillis >= 1000)
	{
		// millis() and prevMillis are both unsigned ints thus the subtraction
		// will always be the absolute value of the difference
		systemTime++;
		prevMillis += 1000;
	}

	return systemTime;
}

void SystemClockClass::setTime(uint32_t time)
{
	systemTime = (uint32_t) time;
	prevMillis = millis();
}

String SystemClockClass::getSystemTimeString()
{
	dateTime.setTime(now());
	return dateTime.toFullDateTimeString();
}

SystemClockClass SystemClock;
