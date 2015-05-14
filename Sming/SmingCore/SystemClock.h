#ifndef APP_SYSTEMCLOCK_H_
#define APP_SYSTEMCLOCK_H_

#include "Clock.h"
#include "../../Services/DateTime/DateTime.h"
#include "../../Wiring/WString.h"

class SystemClockClass
{
public:
	uint32_t now();
	void setTime(uint32_t time);	
	String getSystemTimeString();
	
private:
	uint32_t systemTime;
	uint32_t prevMillis;
	
	DateTime dateTime;
	
};

extern SystemClockClass SystemClock;

#endif /* APP_SYSTEMCLOCK_H_ */
