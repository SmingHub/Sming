#include "../SmingCore/Platform/Rtc.h"

RTC::RTC() {
	rst_info* info = system_get_rst_info();
	hardwareReset = (info->reason == REASON_WDT_RST);
}

uint32_t RTC::getRtcSeconds() {
	RtcData rtcTime;
	loadTime(rtcTime);
	updateRtcTime(rtcTime);
	saveTime(rtcTime);
	return (rtcTime.time / NS_PER_SECOND);
}



bool RTC::setRtcSeconds(uint32_t seconds) {

	RtcData rtcTime;
	loadTime(rtcTime);
	uint64_t offsetNs = ((uint64_t) seconds * NS_PER_SECOND) - rtcTime.time;
	rtcTime.time += offsetNs; // adjust time in nanoseconds by the ntp time offset in nanoseconds
	updateRtcTime(rtcTime);
	return saveTime(rtcTime);
}


void RTC::updateRtcTime(RtcData &data) {

	uint32 rtc_cycles;
	uint32 cal, cal1, cal2;
	cal1 = system_rtc_clock_cali_proc();
	os_delay_us(300);
	cal2 = system_rtc_clock_cali_proc();
	cal = (cal1 + cal2) / 2; // get average cal in case one is out
	rtc_cycles = system_get_rtc_time();

	// hardware reset causes rtc cycles to start at 0 so we need to check and act accordingly
	uint32_t delta = hardwareReset ? rtc_cycles : rtc_cycles - data.cycles;

	hardwareReset = false; // reset hardware flag so we can deal with the next overflow as a reset needs to be dealt differently
	data.time += delta *((uint64) ((cal * 1000) >> 12)); // increment number of nano seconds that have elapsed since last update
	data.cycles = rtc_cycles; // update last number of cycles
}

bool RTC::saveTime(RtcData &data) {
	return system_rtc_mem_write(RTC_DES_ADDR, &data, sizeof(data));
}
void RTC::loadTime(RtcData &data) {

	system_rtc_mem_read(RTC_DES_ADDR, &data, sizeof(data));

	// Initialise the time struct
	if (data.magic != RTC_MAGIC)
	{
		debugf("rtc time init...");
		data.magic = RTC_MAGIC;
		data.time = 0;
		data.cycles = 0;
	}
}


RTC Rtc = RTC();

