#include <SmingCore.h>
#include <Wire.h>
#include <Libraries/DS3232RTC/DS3232RTC.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "ENTER_YOUR_SSID" // Put your SSID and password here
#define WIFI_PWD "ENTER_YOUR_PASSWORD"
#endif

namespace
{
// Change as required
static constexpr uint8_t SDA_PIN{4};
static constexpr uint8_t SCL_PIN{5};

void onNtpReceive(NtpClient& client, time_t timestamp);

NtpClient ntpClient(onNtpReceive);
SimpleTimer printTimer;

void onPrintSystemTime()
{
	DateTime rtcNow = DSRTC.get();
	Serial.println(_F("Current time"));
	Serial << _F("  System (LOCAL TZ): ") << SystemClock.getSystemTimeString() << endl;
	Serial << _F("  UTC (UTC TZ): ") << SystemClock.getSystemTimeString(eTZ_UTC) << endl;
	Serial << _F("  DSRTC (UTC TZ): ") << rtcNow.toFullDateTimeString() << endl;
}

void onNtpReceive(NtpClient& client, time_t timestamp)
{
	// Both System and DSRTC timezones are UTC
	SystemClock.setTime(timestamp, eTZ_UTC);
	DSRTC.set(timestamp);
	// Display LOCAL time
	Serial << _F("Time synchronized: ") << SystemClock.getSystemTimeString() << endl;
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial << _F("Got IP ") << ip << endl;

	ntpClient.requestTime();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println(_F("Sming DSRTC_NTP_SETTER started!"));
	Wire.pins(SDA_PIN, SCL_PIN);
	Wire.begin();

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiEvents.onStationGotIP(gotIP);

	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(2); // GMT+2

	printTimer.initializeMs<2000>(onPrintSystemTime).start();
}
