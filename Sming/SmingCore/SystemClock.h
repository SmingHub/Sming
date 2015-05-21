#ifndef APP_SYSTEMCLOCK_H_
#define APP_SYSTEMCLOCK_H_

#include "Clock.h"
#include "../../Services/DateTime/DateTime.h"
#include "../../Wiring/WString.h"
#include "../SmingCore/Network/NtpClient.h"

class NtpClient;

class SystemClockClass
{
public:
	DateTime now(bool timeUtc = false);
	void setTime(time_t time, bool timeUtc = false);
	String getSystemTimeString(bool timeUtc = false);
	bool setTimezone(double reqTimezone);

	void setNtpSync(String reqServer, int reqInterval);

private:
	uint32_t systemTime;
	uint32_t prevMillis;
	double timezoneDiff = 0.0;
	
	DateTime dateTime;
	NtpClient* ntpClient = nullptr;
	
};

extern SystemClockClass SystemClock;

#endif /* APP_SYSTEMCLOCK_H_ */
