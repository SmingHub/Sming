
#include "SystemClock.h"
#include "Platform/RTC.h"

SystemClockClass SystemClock;

time_t SystemClockClass::now(TimeZone timeType /* = eTZ_Local */)
{
	auto systemTime = RTC.getRtcSeconds();

	if (timeType == eTZ_UTC)
		systemTime -= _tzOffsetSecs;

	return systemTime;
}

bool SystemClockClass::setTime(time_t time, TimeZone timeType /* = eTZ_Local */)
{
	if (timeType == eTZ_UTC)
		time += _tzOffsetSecs;

	bool timeSet = RTC.setRtcSeconds(time);
	if (timeSet)
		_status = eSCS_Set;

	debugf("time updated? %d", timeSet);

	return timeSet;
}

String SystemClockClass::getSystemTimeString(TimeZone timeType /* = eTZ_Local */)
{
	DateTime dt(now(timeType));
	return dt.toFullDateTimeString();
}

bool SystemClockClass::setTimeZoneOffset(int tzOffsetSecs)
{
	if ((unsigned)abs(tzOffsetSecs) < (12 * SECS_PER_HOUR)) {
		_tzOffsetSecs = tzOffsetSecs;
		return true;
	}
	return false;
}

