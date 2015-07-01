#ifndef SMINGCORE_RTC_H_
#define SMINGCORE_RTC_H_

#include "../Wiring/WiringFrameworkDependencies.h"

#define RTC_MAGIC 0x55aaaa55
#define RTC_DES_ADDR 64
#define NS_PER_SECOND 1000000000

typedef struct {
	uint64_t time;
	uint32_t magic;
	uint32_t cycles;
} RtcData;

class RTC
{
public:
	RTC();
	uint32_t getRtcSeconds();
	bool setRtcSeconds(uint32_t seconds);

private:
	bool hardwareReset;
	bool saveToRtcMemory(RtcData &data);
	void updateRtcTime(RtcData &data);
	void readFromRtcMemory(RtcData &data);
};

extern RTC Rtc;
#endif /* SMINGCORE_RTC_H_ */
