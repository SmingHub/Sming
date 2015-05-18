#include "SystemClock.h"

uint32_t SystemClockClass::now(bool timeUtc /* = false */)
{
	// calculate number of seconds passed since last call to now()
	while (millis() - prevMillis >= 1000)
	{
		// millis() and prevMillis are both unsigned ints thus the subtraction
		// will always be the absolute value of the difference
		systemTime++;
		prevMillis += 1000;
	}
    if (timeUtc)
    {
    	return systemTime -  (timezoneDiff * SECS_PER_HOUR); // utc time
    }
    else
    {
    	return systemTime; // local time
    }
}

void SystemClockClass::setTime(uint32_t time, bool timeUtc /* = false */)
{
	systemTime = timeUtc ? (time + (timezoneDiff * SECS_PER_HOUR)) : time;
	prevMillis = millis();
}

String SystemClockClass::getSystemTimeString(bool timeUtc /* = false */)
{
	dateTime.setTime(now(timeUtc));
	return dateTime.toFullDateTimeString();
}

bool SystemClockClass::setTimezone(double reqTimezone)
{
	if ( (reqTimezone >= -12) && (reqTimezone <= 12) )
	{
		timezoneDiff = reqTimezone;
		return true;
	}
	return false;
}

SystemClockClass SystemClock;
