/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "SystemClock.h"
#include "Platform/RTC.h"

SystemClockClass SystemClock;

time_t SystemClockClass::now(TimeZone timeType /* = eTZ_Local */)
{
	uint32_t systemTime = RTC.getRtcSeconds();

	if(timeType == eTZ_UTC) {
		systemTime -= timeZoneOffsetSecs;
	}

	return systemTime;
}

bool SystemClockClass::setTime(time_t time, TimeZone timeType /* = eTZ_Local */)
{
	if(timeType == eTZ_UTC) {
		time += timeZoneOffsetSecs;
	}

	bool timeSet = RTC.setRtcSeconds(time);
	if(timeSet) {
		status = eSCS_Set;
	}

	debugf("time updated? %d", timeSet);

	return timeSet;
}

String SystemClockClass::getSystemTimeString(TimeZone timeType /* = eTZ_Local */)
{
	DateTime dt(now(timeType));
	return DateTime(now(timeType)).toFullDateTimeString();
}

bool SystemClockClass::setTimeZoneOffset(int tzOffsetSecs)
{
	if((unsigned)abs(tzOffsetSecs) < (12 * SECS_PER_HOUR)) {
		timeZoneOffsetSecs = tzOffsetSecs;
		return true;
	}
	return false;
}
