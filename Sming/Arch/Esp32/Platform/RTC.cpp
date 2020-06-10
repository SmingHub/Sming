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

#include "esp_systemapi.h"
#include <soc/rtc.h>
#include <esp32/rom/rtc.h>

RtcClass RTC;

#define RTC_MAGIC 0x55aaaa55
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
static void updateTime();
static void loadTime();

static RTC_DATA_ATTR RtcData rtcTime = {};

RtcClass::RtcClass()
{
	rst_info* info = system_get_rst_info();
	hardwareReset = (info->reason == REASON_WDT_RST);
}

uint64_t RtcClass::getRtcNanoseconds()
{
	loadTime();
	updateTime();
	return rtcTime.time;
}

uint32_t RtcClass::getRtcSeconds()
{
	return (getRtcNanoseconds() / NS_PER_SECOND);
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	loadTime();
	updateTime();
	rtcTime.time = nanoseconds;
	return true;
}

bool RtcClass::setRtcSeconds(uint32_t seconds)
{
	return setRtcNanoseconds((uint64_t)seconds * NS_PER_SECOND);
}

void updateTime()
{
	uint32 rtc_cycles = rtc_time_get();
	uint32 cal = REG_READ(RTC_SLOW_CLK_CAL_REG);

	// hardware reset causes rtc cycles to start at 0 so we need to check and act accordingly
	uint32_t delta = hardwareReset ? rtc_cycles : rtc_cycles - rtcTime.cycles;

	//  reset hardware flag so we can deal with the next overflow as a reset needs to be dealt differently
	hardwareReset = false;
	// increment number of nano seconds that have elapsed since last update
	rtcTime.time += delta * ((uint64)((cal * 1000) >> 12));
	// update last number of cycles
	rtcTime.cycles = rtc_cycles;
}

void loadTime()
{
	// Initialise the time struct
	if(rtcTime.magic != RTC_MAGIC) {
		debugf("rtc time init...");
		rtcTime.magic = RTC_MAGIC;
		rtcTime.time = 0;
		rtcTime.cycles = 0;
	}
}
