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
#include <pico/aon_timer.h>

RtcClass RTC;

#define NS_PER_SECOND 1000000000

void system_init_rtc()
{
	struct timespec ts {
	};
	aon_timer_start(&ts);
}

RtcClass::RtcClass() = default;

uint64_t RtcClass::getRtcNanoseconds()
{
	struct timespec ts;
	return aon_timer_get_time(&ts) ? timespec_to_us(&ts) * 1000ULL : 0;
}

uint32_t RtcClass::getRtcSeconds()
{
	struct timespec ts;
	return aon_timer_get_time(&ts) ? ts.tv_sec : 0;
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	struct timespec ts;
	us_to_timespec(nanoseconds / 1000ULL, &ts);
	return aon_timer_set_time(&ts);
}

bool RtcClass::setRtcSeconds(uint32_t seconds)
{
	struct timespec ts {
		.tv_sec = seconds
	};
	return aon_timer_set_time(&ts);
}
