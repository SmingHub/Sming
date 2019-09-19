/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SystemClock.cpp
 *
 ****/

#include "SystemClock.h"
#include <Platform/RTC.h>
#include <debug_progmem.h>

SystemClockClass SystemClock;

time_t SystemClockClass::now(TimeZone timeType) const
{
	uint32_t systemTime = RTC.getRtcSeconds();

	if(timeType == eTZ_Local) {
		systemTime += timeZoneOffsetSecs;
	}

	return systemTime;
}

bool SystemClockClass::setTime(time_t time, TimeZone timeType)
{
	if(timeType == eTZ_Local) {
		time -= timeZoneOffsetSecs;
	}

	bool timeSet = RTC.setRtcSeconds(time);
	if(timeSet) {
		status = eSCS_Set;
	}

	debugf("time updated? %d", timeSet);

	return timeSet;
}

String SystemClockClass::getSystemTimeString(TimeZone timeType) const
{
	DateTime dt(now(timeType));
	return DateTime(now(timeType)).toFullDateTimeString();
}

bool SystemClockClass::setTimeZoneOffset(int seconds)
{
	if((unsigned)abs(seconds) < (12 * SECS_PER_HOUR)) {
		timeZoneOffsetSecs = seconds;
		return true;
	}
	return false;
}
