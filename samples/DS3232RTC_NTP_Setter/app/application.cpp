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
	Serial.print("Local Time    : ");
	Serial.println(SystemClock.getSystemTimeString());
	Serial.print("UTC Time: ");
	Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));

	Serial.print("dsRTC Time: ");
	DateTime _date_time = DSRTC.get();
	Serial.println(_date_time.toFullDateTimeString());
}

void onNtpReceive(NtpClient& client, time_t timestamp) {
	SystemClock.setTime(timestamp);
	
	DSRTC.set(timestamp);
	
	Serial.print("Time synchronized: ");
	Serial.println(SystemClock.getSystemTimeString());
}

void connectOk()
{
	ntpClient.setAutoQueryInterval(20);
	ntpClient.setAutoQuery(true);
	ntpClient.setAutoUpdateSystemClock(true);
	ntpClient.requestTime();
}

void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void init()
{
	Serial.begin(COM_SPEED_SERIAL); // 115200 by default, change it in Makefile-user.mk
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println("Sming. Let's do smart things!");

	Wire.pins(0, 2); //Change to your SCL,SDA GPIO pin number
        Wire.begin();

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
	
	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	// set timezone hourly difference to UTC
	SystemClock.setTimeZone(3);

	printTimer.initializeMs(2000, onPrintSystemTime).start();
}
