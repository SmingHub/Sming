#include "Platform/RTC.h"

RtcClass RTC;

RtcClass::RtcClass()
{
	rst_info* info = system_get_rst_info();
	_hardwareReset = (info->reason == REASON_WDT_RST);
}

uint64_t RtcClass::getRtcNanoseconds()
{
	RtcData rtcTime;
	loadTime(rtcTime);
	updateTime(rtcTime);
	saveTime(rtcTime);
	return rtcTime.time;
}

bool RtcClass::setRtcNanoseconds(uint64_t nanoseconds)
{
	RtcData rtcTime;
	loadTime(rtcTime);
	updateTime(rtcTime);
	rtcTime.time = nanoseconds;
	return saveTime(rtcTime);
}

void RtcClass::updateTime(RtcData& data)
{
	uint32 rtc_cycles;
	uint32 cal, cal1, cal2;
	cal1 = system_rtc_clock_cali_proc();
	__asm__ __volatile__("memw" : : : "memory");
	// Just for fun
	cal2 = system_rtc_clock_cali_proc();
	cal = (cal1 + cal2) / 2; // get average cal in case one is out
	rtc_cycles = system_get_rtc_time();

	// hardware reset causes rtc cycles to start at 0 so we need to check and act accordingly
	uint32_t delta = _hardwareReset ? rtc_cycles : rtc_cycles - data.cycles;

	// reset hardware flag so we can deal with the next overflow as a reset needs to be dealt differently
	_hardwareReset = false;
	// number of nano seconds that have elapsed since last update
	data.time += delta * ((uint64)((cal * 1000) >> 12));
	// last number of cycles
	data.cycles = rtc_cycles;
}

bool RtcClass::saveTime(RtcData& data)
{
	return system_rtc_mem_write(RTC_DES_ADDR, &data, sizeof(data));
}

void RtcClass::loadTime(RtcData& data)
{
	system_rtc_mem_read(RTC_DES_ADDR, &data, sizeof(data));

	// Initialise the time struct
	if (data.magic != RTC_MAGIC) {
		debugf("rtc time init...");
		data.magic = RTC_MAGIC;
		data.time = 0;
		data.cycles = 0;
	}
}
