#include <user_config.h>
#include <SmingCore/SmingCore.h>


// Put you SSID and Password here
#define WIFI_SSID "ssid"
#define WIFI_PWD "password"

Timer printTimer;

void onPrintSystemTime() {
	Serial.print("Time    : ");
	Serial.println(SystemClock.getSystemTimeString());
	Serial.print("UTC Time:");
	Serial.println(SystemClock.getSystemTimeString(true));
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	// Set SystemClock to do automatic time sync requests every 60 seconds.
	// NOTE: you should have longer interval in a real world application
	// no need for query for time every 60 sec, should be at least 10 minutes or so.
	SystemClock.setNtpSync("pool.ntp.org", 60);

}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}


// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println("Sming. Let's do smart things!");

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	SystemClock.setTimezone(2);
	
	// print time periodically
	printTimer.initializeMs(3000, onPrintSystemTime).start();

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
}
