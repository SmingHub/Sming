#include "SystemClock.h"

DateTime SystemClockClass::now(TimeZone timeType /* = eTZ_Local */)
{
	// calculate number of seconds passed since last call to now()
	while (millis() - prevMillis >= 1000)
	{
		// millis() and prevMillis are both unsigned ints thus the subtraction
		// will always be the absolute value of the difference
		systemTime++;
		prevMillis += 1000;
	}
    if ((timeType == eTZ_Local) || (status == eSCS_Initial))
    {
    	return DateTime(systemTime); // local time
    }
    else
    {
    	return DateTime(systemTime - (timezoneDiff * SECS_PER_HOUR)); // utc time
    }
}

void SystemClockClass::setTime(time_t time, TimeZone timeType /* = eTZ_Local */)
{
	systemTime = (timeType == eTZ_UTC) ? (time + (timezoneDiff * SECS_PER_HOUR)) : time;
	prevMillis = millis();
	status = eSCS_Set;
}

String SystemClockClass::getSystemTimeString(TimeZone timeType /* = eTZ_Local */)
{
	dateTime.setTime(now(timeType));
	return dateTime.toFullDateTimeString();
}

bool SystemClockClass::setTimeZone(double localTimezone)
{
	if ( (localTimezone >= -12) && (localTimezone <= 12) )
	{
		timezoneDiff = localTimezone;
		return true;
	}
	return false;
}

SystemClockClass SystemClock;
