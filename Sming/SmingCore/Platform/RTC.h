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

class RtcClass
{
public:
	RtcClass();
	uint32_t getRtcSeconds();
	bool setRtcSeconds(uint32_t seconds);

private:
	bool hardwareReset;
	bool saveTime(RtcData &data);
	void updateTime(RtcData &data);
	void loadTime(RtcData &data);
};

extern RtcClass RTC;
#endif /* SMINGCORE_RTC_H_ */
