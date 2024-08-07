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
#include <sys/time.h>

extern "C" int settimeofday(const struct timeval*, const struct timezone*);

RtcClass RTC;

#define NS_PER_SECOND 1000000000

void system_init_rtc()
{
	rtc_init();
	datetime_t t{.year = 1970, .month = 1, .day = 1};
	rtc_set_datetime(&t);
}

RtcClass::RtcClass() = default;

uint64_t RtcClass::getRtcNanoseconds()
{
	return uint64_t(getRtcSeconds()) * NS_PER_SECOND;
}

uint32_t RtcClass::getRtcSeconds()
{
	datetime_t t;
	if(!rtc_get_datetime(&t)) {
		return 0;
	}

	DateTime dt;
	dt.setTime(t.sec, t.min, t.hour, t.day, t.month - 1, t.year);

	return time_t(dt);
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	return setRtcSeconds(nanoseconds / NS_PER_SECOND);
}

bool RtcClass::setRtcSeconds(uint32_t seconds)
{
	struct timeval tv {
		seconds
	};
	settimeofday(&tv, nullptr);

	DateTime dt{seconds};

	datetime_t t = {
		.year = int16_t(dt.Year),
		.month = int8_t(1 + dt.Month),
		.day = int8_t(dt.Day),
		.dotw = int8_t(dt.DayofWeek),
		.hour = int8_t(dt.Hour),
		.min = int8_t(dt.Minute),
		.sec = int8_t(dt.Second),
	};

	return rtc_set_datetime(&t);
}
