#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

FTPServer ftp;

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("\r\nI'm CONNECTED:");
	Serial.println(WifiStation.getIP().toString());

	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
	// You can also use special FTP comand: "fsformat" for clearing file system (for example from TotalCMD)
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help!!! :(");

	// .. some you code for configuration ..
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

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
