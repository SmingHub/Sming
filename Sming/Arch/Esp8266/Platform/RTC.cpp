/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RTC.cpp
 *
 ****/

#include "Platform/RTC.h"

RtcClass RTC;

#define RTC_MAGIC 0x55aaaa55
#define RTC_DES_ADDR 64
#define NS_PER_SECOND 1000000000

/** @brief  Structure to hold RTC data
 *  @addtogroup structures
 */
typedef struct {
	uint64_t time;   ///< Quantity of nanoseconds since epoch
	uint32_t magic;  ///< Magic ID used to identify that RTC has been initialised
	uint32_t cycles; ///< Quantity of RTC cycles since last update
} RtcData;

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
	uint32 rtc_cycles;
	uint32 cal, cal1, cal2;
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
		debugf("rtc time init...");
		data.magic = RTC_MAGIC;
		data.time = 0;
		data.cycles = 0;
	}
}
