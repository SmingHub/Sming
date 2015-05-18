#ifndef APP_SYSTEMCLOCK_H_
#define APP_SYSTEMCLOCK_H_

#include "Clock.h"
#include "../../Services/DateTime/DateTime.h"
#include "../../Wiring/WString.h"

class SystemClockClass
{
public:
	uint32_t now(bool timeUtc = false);
	void setTime(uint32_t time, bool timeUtc = false);
	String getSystemTimeString(bool timeUtc = false);
	bool setTimezone(double reqTimezone);

private:
	uint32_t systemTime;
	uint32_t prevMillis;
	double timezoneDiff = 0.0;
	
	DateTime dateTime;
	
};

extern SystemClockClass SystemClock;

#endif /* APP_SYSTEMCLOCK_H_ */
