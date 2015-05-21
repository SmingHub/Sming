#include <user_config.h>
#include <SmingCore/SmingCore.h>


// Put you SSID and Password here
#define WIFI_SSID "ssid"
#define WIFI_PWD "pw"


void onNtpReceive(NtpClient& client, time_t timestamp);

Timer printTimer;
//NtpClient ntpClient(onNtpReceive);
//NtpClient ntpc("fritz.box",30, onNtpReceive);


void onPrintSystemTime() {
	Serial.print("Time    : ");
	Serial.println(SystemClock.getSystemTimeString());
	Serial.print("UTC Time:");
	Serial.println(SystemClock.getSystemTimeString(true));
}


// Called when time has been received by NtpClient.
// Either after manual requestTime() or when
// and automatic request has been made.
void onNtpReceive(NtpClient& client, time_t timestamp) {
	SystemClock.setTime(timestamp);

	Serial.print("Time synchronized: ");
	Serial.println(SystemClock.getSystemTimeString());
	client.setNtpServer("pool.ntp.org");
}

NtpClient *nt;

// Will be called when WiFi station was connected to AP
void connectOk()
{
	// Set client to do automatic time requests every 60 seconds.
	// NOTE: you should have longer interval in a real world application
	// no need for query for time every 60 sec, should be at least 10 minutes or so.
//	ntpClient.setAutoQueryInterval(30);
//	ntpClient.setAutoQuery(true);

	// Request to update time now. 
	// Otherwise the set interval will pass before time
	// is updated.
//	ntpClient.requestTime();
	SystemClock.setNtpSync("fritz.box",30);
//	nt = new NtpClient("fritz.box",30);
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
	Serial.begin(74880);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println("Sming. Let's do smart things!");

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	SystemClock.setTimezone(2);

	printTimer.initializeMs(1000, onPrintSystemTime).start();
	
	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
}
