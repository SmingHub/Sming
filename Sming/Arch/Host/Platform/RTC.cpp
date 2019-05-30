/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RTC.cpp
 *
 ****/

#include <Platform/RTC.h>

#include <sys/time.h>

RtcClass RTC;

RtcClass::RtcClass()
{
}

uint64_t RtcClass::getRtcNanoseconds()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	uint64_t usecs = (tv.tv_sec * 1000000ULL) + (uint32_t)tv.tv_usec;
	return usecs * 1000;
}

uint32_t RtcClass::getRtcSeconds()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return tv.tv_sec;
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	return false;
}

bool RtcClass::setRtcSeconds(uint32_t seconds)
{
	return false;
}
