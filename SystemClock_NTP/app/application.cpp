#include <user_config.h>
#include <SmingCore/SmingCore.h>


// Put you SSID and Password here
#define WIFI_SSID "PleaseEnterSSID"
#define WIFI_PWD "PleaseEnterPass"


void onNtpReceive(NtpClient& client, time_t timestamp);

Timer printTimer;

// Option 1
// Use this option if you want to have full control of NtpTime client
// Default : no automatic NTP requests sent, no automatic update of SystemTime
// Default : NTP_DEFAULT_SERVER and NTP_DEFAULT_AUTO_UPDATE_INTERVAL from ntpclient.h

// NtpClient ntpClient(onNtpReceive);

// Option 2
// Use this option if you want control but other server/timeout options
// Default : automatic NTP requests are sent, no automatic update of SystemTime

// NtpClient ntpClient_myoptins ("my_ntp_server",myrefreshinterval, onNtpReceive);

// Option 3
// Use this option if you want to start wit autorefresh and autosystemupdate
// No further callback from ntpClient
NtpClient ntpClient("pool.ntp.org", 30);

// Option 4
// only create pointer and initialize on ConnectOK
// NtpClient *ntpClient;

void onPrintSystemTime() {
	Serial.print("Time    : ");
	Serial.println(SystemClock.getSystemTimeString());
	Serial.print("UTC Time: ");
	Serial.println(SystemClock.getSystemTimeString(true));
}


// Called when time has been received by NtpClient (option 1 or 2)
// Either after manual requestTime() or when
// and automatic request has been made.
void onNtpReceive(NtpClient& client, time_t timestamp) {
	SystemClock.setTime(timestamp);

	Serial.print("Time synchronized: ");
	Serial.println(SystemClock.getSystemTimeString());
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
// Set specific parameters if started by option 1 or 2
	// Set client to do automatic time requests every 60 seconds.
	// NOTE: you should have longer interval in a real world application
	// no need for query for time every 60 sec, should be at least 10 minutes or so.
//	ntpClient.setAutoQueryInterval(60);
//	ntpClient.setAutoQuery(true);
//  ntpClient.setAutoUpdateSystemClock(true);
	// Request to update time now. 
	// Otherwise the set interval will pass before time
	// is updated.
//	ntpClient.requestTime();

//  When using option 4 -> create client after connect OK
//  ntpClient = new NtpClient("my_ntp_server", myrefreshinterval);

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

	// set timezone to hourly difference to UTC
	SystemClock.setTimezone(2);

	printTimer.initializeMs(1000, onPrintSystemTime).start();
	
	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start
}
