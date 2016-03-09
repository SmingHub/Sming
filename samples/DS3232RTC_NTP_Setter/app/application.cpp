#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <SmingCore/Wire.h>
#include <Libraries/DS3232RTC/DS3232RTC.h>


// Connect DS3232 / DS3231 to GND, VCC, GPIO0 - SCL, GPIO2 - SDA or any other free GPIO and change below Wire.pins(0,2)
// to you SCL and SDA pins

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
	Serial.printf("Current time\nSystem(LOCAL TZ): %s\nUTC(UTC TZ): %s\nDSRTC(UTC TZ): %s\n\n", SystemClock.getSystemTimeString().c_str(),
								SystemClock.getSystemTimeString(eTZ_UTC).c_str(),
								_date_time.toFullDateTimeString().c_str());
}

void onNtpReceive(NtpClient& client, time_t timestamp) {
	SystemClock.setTime(timestamp, eTZ_UTC); //System timezone is LOCAL so to set it from UTC we specify TZ
	DSRTC.set(timestamp); //DSRTC timezone is UTC so we need TZ-correct DSRTC.get()
	Serial.printf("Time synchronized: %s\n", SystemClock.getSystemTimeString().c_str());
}

void connectOk()
{
	ntpClient.requestTime();
}

void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println("Sming DSRTC_NTP_SETTER started!");
//Uncomment proper line, or set right pins by hand
	Wire.pins(0, 2); //Change to your SCL - 0,SDA - 2 GPIO pin number
//	Wire.pins(5, 4); //Change to your SCL - 5,SDA - 4 GPIO pin number
        Wire.begin();

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
	
	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(2); // GMT+2

	printTimer.initializeMs(2000, onPrintSystemTime).start();
}
