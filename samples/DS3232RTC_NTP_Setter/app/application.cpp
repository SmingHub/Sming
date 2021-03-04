#include <SmingCore.h>

#include <Wire.h>
#include <Libraries/DS3232RTC/DS3232RTC.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "ENTER_YOUR_SSID" // Put your SSID and password here
#define WIFI_PWD "ENTER_YOUR_PASSWORD"
#endif

// Change as required
static constexpr uint8_t SDA_PIN{4};
static constexpr uint8_t SCL_PIN{5};

void onNtpReceive(NtpClient& client, time_t timestamp);

Timer printTimer;

NtpClient ntpClient(onNtpReceive);

void onPrintSystemTime()
{
	DateTime rtcNow = DSRTC.get();
	Serial.println(_F("Current time"));
	Serial.print(_F("  System(LOCAL TZ): "));
	Serial.println(SystemClock.getSystemTimeString());
	Serial.print(_F("  UTC(UTC TZ): "));
	Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));
	Serial.print(_F("  DSRTC(UTC TZ): "));
	Serial.println(rtcNow.toFullDateTimeString());
}

void onNtpReceive(NtpClient& client, time_t timestamp)
{
	SystemClock.setTime(timestamp, eTZ_UTC); //System timezone is LOCAL so to set it from UTC we specify TZ
	DSRTC.set(timestamp);					 //DSRTC timezone is UTC so we need TZ-correct DSRTC.get()
	Serial.print(_F("Time synchronized: "));
	Serial.println(SystemClock.getSystemTimeString());
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	debug_i("Got IP %s", ip.toString().c_str());

	ntpClient.requestTime();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println("Sming DSRTC_NTP_SETTER started!");
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
