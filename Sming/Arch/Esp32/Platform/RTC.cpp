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

// #include <ESP_VARIANT/clk.h>
extern "C" uint64_t esp_clk_rtc_time(void);

RtcClass RTC;

namespace
{
constexpr uint64_t US_PER_SECOND{1000000};
constexpr uint64_t NS_PER_SECOND{US_PER_SECOND * 1000};
uint64_t clockOffset;

} // namespace

RtcClass::RtcClass() = default;

uint64_t RtcClass::getRtcNanoseconds()
{
	return (clockOffset + esp_clk_rtc_time()) * 1000ULL;
}

uint32_t RtcClass::getRtcSeconds()
{
	return (clockOffset + esp_clk_rtc_time()) / US_PER_SECOND;
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	clockOffset = (nanoseconds / 1000) - esp_clk_rtc_time();
	return true;
}

bool RtcClass::setRtcSeconds(uint32_t seconds)
{
	struct timeval tv {
		seconds
	};
	settimeofday(&tv, nullptr);

	return setRtcNanoseconds(uint64_t(seconds) * NS_PER_SECOND);
}
