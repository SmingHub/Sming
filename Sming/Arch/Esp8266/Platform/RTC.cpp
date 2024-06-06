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
#include <esp_systemapi.h>
#include <sys/time.h>
#include <errno.h>

RtcClass RTC;

#define RTC_MAGIC 0x55aaaa55
#define RTC_DES_ADDR 64 + 3 ///< rBoot may require 3 words at start
#define NS_PER_SECOND 1'000'000'000

/** @brief  Structure to hold RTC data
 *  @addtogroup structures
 */
struct RtcData {
	uint64_t time;   ///< Quantity of nanoseconds since epoch
	uint32_t magic;  ///< Magic ID used to identify that RTC has been initialised
	uint32_t cycles; ///< Quantity of RTC cycles since last update
};

static bool hardwareReset;
static bool saveTime(RtcData& data);
static void updateTime(RtcData& data);
static void loadTime(RtcData& data);

RtcClass::RtcClass()
{
	rst_info* info = system_get_rst_info();
	hardwareReset = (info->reason == REASON_WDT_RST);
}

uint64_t RtcClass::getRtcNanoseconds()
{
	RtcData rtcTime;
	loadTime(rtcTime);
	updateTime(rtcTime);
	saveTime(rtcTime);
	return rtcTime.time;
}

uint32_t RtcClass::getRtcSeconds()
{
	return (getRtcNanoseconds() / NS_PER_SECOND);
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	RtcData rtcTime;
	loadTime(rtcTime);
	updateTime(rtcTime);
	rtcTime.time = nanoseconds;
	return saveTime(rtcTime);
}

bool RtcClass::setRtcSeconds(uint32_t seconds)
{
	return setRtcNanoseconds((uint64_t)seconds * NS_PER_SECOND);
}

void updateTime(RtcData& data)
{
	uint32_t rtc_cycles;
	uint32_t cal, cal1, cal2;
	cal1 = system_rtc_clock_cali_proc();
	__asm__ __volatile__("memw" : : : "memory"); // Just for fun
	cal2 = system_rtc_clock_cali_proc();
	cal = (cal1 + cal2) / 2; // get average cal in case one is out
	rtc_cycles = system_get_rtc_time();

	// hardware reset causes rtc cycles to start at 0 so we need to check and act accordingly
	uint32_t delta = hardwareReset ? rtc_cycles : rtc_cycles - data.cycles;

	//  reset hardware flag so we can deal with the next overflow as a reset needs to be dealt differently
	hardwareReset = false;
	// increment number of nano seconds that have elapsed since last update
	data.time += delta * ((uint64)((cal * 1000) >> 12));
	// update last number of cycles
	data.cycles = rtc_cycles;
}

bool saveTime(RtcData& data)
{
	return system_rtc_mem_write(RTC_DES_ADDR, &data, sizeof(data));
}

void loadTime(RtcData& data)
{
	system_rtc_mem_read(RTC_DES_ADDR, &data, sizeof(data));

	// Initialise the time struct
	if(data.magic != RTC_MAGIC) {
		debug_d("rtc time init...");
		data.magic = RTC_MAGIC;
		data.time = 0;
		data.cycles = 0;
	}
}

extern "C" int settimeofday(const struct timeval* tv, const struct timezone* tz)
{
	// os_printf_plus("** settimeofday(%p, %p), secs = %u\r\n", tv, tz, tv ? unsigned(tv->tv_sec) : 0);

	// Received from lwip2 SNTP
	const uint32_t LWIP2_SNTP_MAGIC = 0xfeedC0de;

	if(reinterpret_cast<uint32_t>(tz) == LWIP2_SNTP_MAGIC) {
		tz = nullptr;
	}

	if(tz || !tv) {
		// tz is obsolete (cf. man settimeofday)
		return EINVAL;
	}

	// lwip2 calls this during static initialisation, before RTC is initialised, so ignore value 0
	if(tv->tv_sec) {
		uint64_t ns = uint64_t(tv->tv_sec) * NS_PER_SECOND + tv->tv_usec * 1000;
		RTC.setRtcNanoseconds(ns);
	}
	return 0;
}

extern "C" int _gettimeofday_r(struct _reent*, struct timeval* tp, void*)
{
	if(tp) {
		uint32_t micros = RTC.getRtcNanoseconds() / 1000LL;
		tp->tv_sec = micros / 1000;
		tp->tv_usec = micros % 1000;
	}
	return 0;
}

extern "C" time_t time(time_t* t)
{
	time_t seconds = RTC.getRtcSeconds();
	if(t) {
		*t = seconds;
	}
	return seconds;
}
