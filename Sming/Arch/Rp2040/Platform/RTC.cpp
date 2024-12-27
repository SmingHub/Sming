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
#include <sys/time.h>
#include <pico/time.h>

RtcClass RTC;

#define NS_PER_SECOND 1'000'000'000
#define US_PER_SECOND 1'000'000

RtcClass::RtcClass() = default;

namespace
{
int64_t epoch_sys_time_us;
}

extern "C" int _gettimeofday(struct timeval* tv, void*)
{
	if(tv) {
		auto us_since_epoch = epoch_sys_time_us + time_us_64();
		*tv = {
			.tv_sec = time_t(us_since_epoch / US_PER_SECOND),
			.tv_usec = suseconds_t(us_since_epoch % US_PER_SECOND),
		};
	}
	return 0;
}

extern "C" int settimeofday(const struct timeval* tv, const struct timezone*)
{
	if(tv) {
		auto us_since_epoch = tv->tv_sec * US_PER_SECOND + tv->tv_usec;
		epoch_sys_time_us = us_since_epoch - time_us_64();
	}
	return 0;
}

uint64_t RtcClass::getRtcNanoseconds()
{
	return uint64_t(epoch_sys_time_us + time_us_64()) * 1000ULL;
}

uint32_t RtcClass::getRtcSeconds()
{
	return (epoch_sys_time_us + time_us_64()) / US_PER_SECOND;
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	auto us_since_epoch = nanoseconds / 1000;
	epoch_sys_time_us = us_since_epoch - get_absolute_time();
	return true;
}

bool RtcClass::setRtcSeconds(uint32_t seconds)
{
	auto us_since_epoch = int64_t(seconds) * US_PER_SECOND;
	epoch_sys_time_us = us_since_epoch - time_us_64();
	return true;
}
