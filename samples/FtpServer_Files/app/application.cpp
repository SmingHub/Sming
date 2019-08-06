#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

FtpServer ftp;

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	Serial.printf("IP: %s\n", ip.toString().c_str());
	// Start FTP server
	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account
	// You can also use special FTP comand: "fsformat" for clearing file system (for example from TotalCMD)
}

// Will be called when WiFi station timeout was reached
void connectFail(const String& ssid, const MACAddress& bssid, WifiDisconnectReason reason)
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
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
