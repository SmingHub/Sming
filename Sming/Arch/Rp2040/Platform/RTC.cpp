/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RTC.cpp
 *
 ****/

#include <Platform/RTC.h>
#include <DateTime.h>
#include <hardware/rtc.h>

RtcClass RTC;

#define NS_PER_SECOND 1000000000

namespace
{
bool initialised;

void checkInit()
{
	if(initialised) {
		return;
	}
	rtc_init();
	initialised = true;
}

} // namespace

RtcClass::RtcClass()
{
	rtc_init();
}

uint64_t RtcClass::getRtcNanoseconds()
{
	return uint64_t(getRtcSeconds()) * NS_PER_SECOND;
}

uint32_t RtcClass::getRtcSeconds()
{
	checkInit();

	datetime_t t;
	rtc_get_datetime(&t);

	DateTime dt;
	dt.setTime(t.sec, t.min, t.hour, t.day, t.month, t.year);

	return time_t(dt);
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	return setRtcSeconds(nanoseconds / NS_PER_SECOND);
}

bool RtcClass::setRtcSeconds(uint32_t seconds)
{
	checkInit();

	DateTime dt{seconds};

	datetime_t t = {
		.year = int16_t(dt.Year),
		.month = int8_t(dt.Month),
		.day = int8_t(dt.Day),
		.dotw = int8_t(dt.DayofWeek),
		.hour = int8_t(dt.Hour),
		.min = int8_t(dt.Minute),
		.sec = int8_t(dt.Second),
	};

	return rtc_set_datetime(&t);
}
