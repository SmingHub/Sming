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
#include "ZonedTime.h"
#include <Platform/RTC.h>
#include <debug_progmem.h>

SystemClockClass SystemClock;

time_t SystemClockClass::now(TimeZone timeType) const
{
	uint32_t systemTime = RTC.getRtcSeconds();

	if(timeType == eTZ_Local) {
		if(checkTimeZoneOffset) {
			checkTimeZoneOffset(systemTime);
		}
		systemTime += zoneInfo.offsetSecs();
	}

	return systemTime;
}

bool SystemClockClass::setTime(time_t time, TimeZone timeType)
{
	if(timeType == eTZ_Local) {
		time -= zoneInfo.offsetSecs();
	}

	timeSet = RTC.setRtcSeconds(time);

	debugf("time updated? %d", timeSet);

	return timeSet;
}

String SystemClockClass::getSystemTimeString(TimeZone timeType) const
{
	time_t systemTime = now(eTZ_UTC);
	if(checkTimeZoneOffset) {
		checkTimeZoneOffset(systemTime);
	}

	if(timeType == eTZ_UTC) {
		DateTime dt(systemTime);
		return dt.toFullDateTimeString();
	}

	ZonedTime time(systemTime, zoneInfo);
	return time.toString();
}
