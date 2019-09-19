#include <NtpClientDemo.h>
#include <SolarCalculator.h>

const TimeChangeRule NtpClientDemo::dstStart = {"BST", Last, Sun, Mar, 1, 60};
const TimeChangeRule NtpClientDemo::stdStart = {"GMT", Last, Sun, Oct, 2, 0};
Timezone NtpClientDemo::tz(dstStart, stdStart);

void NtpClientDemo::ntpResult(NtpClient& client, time_t ntpTime)
{
	/*
	 * Update the system clock and calculate the correct time offset,
	 * accounting for time zone and daylight savings.
	 */
	auto localTime = tz.toLocal(ntpTime);
	SystemClock.setTime(ntpTime, eTZ_UTC);
	SystemClock.setTimeZoneOffset(localTime - ntpTime);

	/*
	 * Display the new time
	 */
	Serial.print("ntpClientDemo Callback: ntpTime = ");
	Serial.print(ntpTime);
	Serial.print(", ");
	Serial.print(SystemClock.getSystemTimeString(eTZ_UTC));
	Serial.print(" UTC, Local time = ");
	Serial.print(SystemClock.getSystemTimeString(eTZ_Local));
	Serial.print(" ");
	Serial.println(tz.utcTimeTag(ntpTime));

	/*
	 * Display times of next sunrise and sunset
	 */
	DateTime sunrise = getNextSunriseSet(true);
	DateTime sunset = getNextSunriseSet(false);
	Serial.print("Next sunrise at ");
	Serial.print(sunrise.toShortTimeString());
	Serial.print(", sunset at ");
	Serial.println(sunset.toShortTimeString());
}

/*
 * We use the y/m/d from local time for sunrise/sunset calculations, and the solar calculator
 * returns the time from midnight in UTC for that day. We therefore need to adjust this
 * to account for timezone and daylight savings.
 */
time_t NtpClientDemo::getNextSunriseSet(bool isSunrise)
{
	auto timeNow = SystemClock.now(eTZ_Local);
	DateTime dt(timeNow);
	dt.Hour = 0;
	dt.Minute = 0;
	dt.Second = 0;
	SolarCalculator calc;
	int offset_secs = SECS_PER_MIN * calc.sunRiseSet(isSunrise, dt.Year, dt.Month + 1, dt.Day);

	time_t t = tz.toLocal(dt + offset_secs);

	// If time has already passed, then make it tomorrow
	if(t < timeNow) {
		t = tz.toLocal(dt + offset_secs + SECS_PER_DAY);
	}

	return t;
}
