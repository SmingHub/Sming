#include <user_config.h>
#include <SmingCore/SmingCore.h>

// Put you SSID and Password here
//#define WIFI_SSID "EnterSSID"
//#define WIFI_PWD "EnterPassword"

#define WIFI_SSID "DontConnect"
#define WIFI_PWD "ignoreme"

FTPServer ftp;

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("\r\nI'm CONNECTED:");
	Serial.println(WifiStation.getIP().toString());

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help!!! :(");

	// .. some you code for configuration ..
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	fileSetContent("example.txt", "hello world!");
	fileSetContent("data.bin", "\1\2\3\4\5");

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
}
