#include "SystemClock.h"


DateTime SystemClockClass::now(TimeZone timeType /* = eTZ_Local */)
{
	uint32_t systemTime = RTC.getRtcSeconds();

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
	bool timeSet =
	(timeType == eTZ_UTC) ?	RTC.setRtcSeconds((time + (timezoneDiff * SECS_PER_HOUR))) : RTC.setRtcSeconds(time);
	debugf("time updated? %d", timeSet);
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
