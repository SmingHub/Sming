#include <NtpClientDemo.h>
#include <SystemClock.h>
#include <HardwareSerial.h>
#include <SolarCalculator.h>
#include <Timezone.h>
#include <tzdata.h>

namespace MyZone
{
using namespace TZ;
/*
 * For handling local/UTC time conversions
 * This is for the UK, amend as required
 */
const Rule dstStart{"BST", Last, Sun, Mar, 1, 60};
const Rule stdStart{"GMT", Last, Sun, Oct, 2, 0};

// Posix rule string
DEFINE_FSTR_LOCAL(tzstr, "GMT0BST,M3.5.0/1,M10.5.0")

// Sunrise/sunset requires co-ordinates
SolarRef solarRef = {51.4769, 0.0005}; // Greenwich, London

} // namespace MyZone

namespace
{
// We can initialise timezone in various ways
// Timezone tz = Timezone::fromPosix(MyZone::tzstr);
// Timezone tz(MyZone::dstStart, MyZone::stdStart);
Timezone tz = TZ::Europe::London();

/*
 * We use the y/m/d from local time for sunrise/sunset calculations, and the solar calculator
 * returns the time from midnight in UTC for that day. We therefore need to adjust this
 * to account for timezone and daylight savings.
 */
ZonedTime getNextSunriseSet(bool isSunrise)
{
	auto utcNow = SystemClock.now(eTZ_UTC);
	DateTime dt(tz.toLocal(utcNow));
	dt.Hour = 0;
	dt.Minute = 0;
	dt.Second = 0;
	SolarCalculator calc(MyZone::solarRef);
	int offset_secs = SECS_PER_MIN * calc.sunRiseSet(isSunrise, dt.Year, dt.Month + 1, dt.Day);

	time_t utcNext = dt + offset_secs;

	// If time has already passed, then make it tomorrow
	if(utcNext < utcNow) {
		utcNext += SECS_PER_DAY;
	}

	return tz.makeZoned(utcNext);
}

void checkTimeZoneOffset(time_t systemTime)
{
	static ZonedTime nextChange{TZ::invalidTime};

	if(nextChange == TZ::invalidTime) {
		nextChange = tz.makeZoned(systemTime);
	} else if(systemTime < nextChange) {
		return;
	}

	SystemClock.setTimeZone(nextChange.getZoneInfo());
	nextChange = tz.getNextChange(systemTime);
}

} // namespace

void NtpClientDemo::ntpResult(NtpClient& client, time_t ntpTime)
{
	/*
	 * Update the system clock and calculate the correct time offset,
	 * accounting for time zone and daylight savings.
	 */
	SystemClock.setTime(ntpTime, eTZ_UTC);

	// Now we've set the clock, we can determine the initial active timezone and maintain the offset
	SystemClock.onCheckTimeZoneOffset([this](time_t systemTime) { checkTimeZoneOffset(systemTime); });

	/*
	 * Display the new time
	 */
	Serial << _F("ntpClientDemo Callback: ntpTime = ") << ntpTime << ", " << SystemClock.getSystemTimeString(eTZ_UTC)
		   << _F(" UTC, Local time = ") << SystemClock.getSystemTimeString(eTZ_Local) << ' ' << tz.utcTimeTag(ntpTime)
		   << endl;

	/*
	 * Display times of next sunrise and sunset
	 */
	ZonedTime sunrise = getNextSunriseSet(true);
	ZonedTime sunset = getNextSunriseSet(false);
	Serial << _F("Next sunrise ") << sunrise.toString() << _F(", sunset ") << sunset.toString() << endl;

	/*
	 * Display points at which daylight savings changes.
	 */
	if(!tz.hasDaylightSavings()) {
		Serial << _F("Selected timezone has no daylight savings.") << endl;
		return;
	}

	DateTime dt(ntpTime);

	Serial << _F("In ") << dt.Year << _F(", daylight savings:") << endl;
	ZonedTime dst = tz.getTransition(dt.Year, true);
	ZonedTime transition = tz.makeZoned(dst, true);
	Serial << _F("  Starts: ") << transition.toString() << " (" << dst.toUtc().toString() << ")" << endl;

	ZonedTime std = tz.getTransition(dt.Year, false);
	transition = tz.makeZoned(std, true);
	Serial << _F("    Ends: ") << transition.toString() << " (" << std.toUtc().toString() << ")" << endl;

	ZonedTime nextChange = tz.getNextChange(ntpTime);
	Serial << _F("Next change to ") << nextChange.tag() << _F(" on ") << nextChange.toUtc().toString() << endl;
}
