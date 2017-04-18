#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <SmingCore/Wire.h>
#include <Libraries/DS3232RTC/DS3232RTC.h>


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "ENTER_YOUR_SSID" // Put you SSID and Password here
	#define WIFI_PWD "ENTER_YOUR_PASSWORD"
#endif

void onNtpReceive(NtpClient& client, time_t timestamp);

Timer printTimer;

NtpClient ntpClient ("pool.ntp.org", 30, onNtpReceive);

void onPrintSystemTime() {
	DateTime _date_time = DSRTC.get();
	Serial.printf("Current time\n\tSystem(LOCAL TZ): %s\n\tUTC(UTC TZ): %s\n\tDSRTC(UTC TZ): %s\n\n",
		SystemClock.getSystemTimeString().c_str(),
		SystemClock.getSystemTimeString(eTZ_UTC).c_str(),
		_date_time.toFullDateTimeString().c_str()
	);
}

void onNtpReceive(NtpClient& client, time_t timestamp) {
	SystemClock.setTime(timestamp, eTZ_UTC); //System timezone is LOCAL so to set it from UTC we specify TZ
	DSRTC.set(timestamp); //DSRTC timezone is UTC so we need TZ-correct DSRTC.get()
	Serial.printf("Time synchronized: %s\n", SystemClock.getSystemTimeString().c_str());
}

void systemReady()
{
	ntpClient.requestTime();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.println("Sming DSRTC_NTP_SETTER started!");
	Wire.pins(0, 2); //Change to your SCL - 0,SDA - 2 GPIO pin number
	Wire.begin();

	// Station - WiFi client
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
	WifiStation.enable(true);

	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(2); // GMT+2

	System.onReady(systemReady);

	printTimer.initializeMs(2000, onPrintSystemTime).start();
}
