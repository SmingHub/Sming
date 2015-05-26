#ifndef APP_SYSTEMCLOCK_H_
#define APP_SYSTEMCLOCK_H_

#include "Clock.h"
#include "../../Services/DateTime/DateTime.h"
#include "../../Wiring/WString.h"
#include "../SmingCore/Network/NtpClient.h"

typedef enum
{
	eSCUtc 		= 0,
	eSCLocal	= 1
} eSysClockTime;

typedef enum
{
	eSCInitial	= 0,
	eSCSet		= 1
} eSysClockStatus;

class NtpClient;

class SystemClockClass
{
public:
	DateTime now(eSysClockTime timeType = eSCLocal);
	void setTime(time_t time, eSysClockTime timeType = eSCLocal);
	String getSystemTimeString(eSysClockTime timeType = eSCLocal);
	bool setTimezone(double reqTimezone);

	void setNtpSync(String reqServer, int reqInterval);

	NtpClient* ntpClient = nullptr;
private:
	uint32_t systemTime;
	uint32_t prevMillis;
	double timezoneDiff = 0.0;
	
	DateTime dateTime;

	eSysClockStatus SCStatus = eSCInitial;
	
};

extern SystemClockClass SystemClock;

#endif /* APP_SYSTEMCLOCK_H_ */
