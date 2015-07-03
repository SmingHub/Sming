#ifndef APP_SYSTEMCLOCK_H_
#define APP_SYSTEMCLOCK_H_

#include "Clock.h"
#include "../../Services/DateTime/DateTime.h"
#include "../../Wiring/WString.h"
#include "../SmingCore/Network/NtpClient.h"
#include "../SmingCore/Platform/RTC.h"

enum TimeZone
{
	eTZ_UTC		= 0,
	eTZ_Local	= 1
};

enum SystemClockStatus
{
	eSCS_Initial	= 0,
	eSCS_Set		= 1
};

class NtpClient;

class SystemClockClass
{
public:
	DateTime now(TimeZone timeType = eTZ_Local);
	void setTime(time_t time, TimeZone timeType = eTZ_Local);
	String getSystemTimeString(TimeZone timeType = eTZ_Local);
	bool setTimeZone(double localTimezoneOffset);

private:
	double timezoneDiff = 0.0;
	DateTime dateTime;
	SystemClockStatus status = eSCS_Initial;
};

extern SystemClockClass SystemClock;

#endif /* APP_SYSTEMCLOCK_H_ */
