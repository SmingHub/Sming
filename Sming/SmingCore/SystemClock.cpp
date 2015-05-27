#include "SystemClock.h"

DateTime SystemClockClass::now(eSysClockTime timeType /* = eSCLocal */)
{
	// calculate number of seconds passed since last call to now()
	while (millis() - prevMillis >= 1000)
	{
		// millis() and prevMillis are both unsigned ints thus the subtraction
		// will always be the absolute value of the difference
		systemTime++;
		prevMillis += 1000;
	}
    if ((timeType == eSCLocal) || (SCStatus == eSCInitial))
    {
    	return DateTime(systemTime); // local time
    }
    else
    {
    	return DateTime(systemTime -  (timezoneDiff * SECS_PER_HOUR)); // utc time
    }
}

void SystemClockClass::setTime(time_t time, eSysClockTime timeType /* = eSCLocal */)
{
	systemTime = (timeType == eSCLocal) ? (time + (timezoneDiff * SECS_PER_HOUR)) : time;
	prevMillis = millis();
	SCStatus = eSCSet;
}

String SystemClockClass::getSystemTimeString(eSysClockTime timeType /* = eSCLocal */)
{
	dateTime.setTime(now(timeType));
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
