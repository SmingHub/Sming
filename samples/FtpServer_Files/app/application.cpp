#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

FtpServer ftp;

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial << "IP: " << ip << endl;
	// Start FTP server
	ftp.listen(21);
	// Add user accounts
	ftp.addUser("guest", nullptr, IFS::UserRole::Guest);
	ftp.addUser("me", "123", IFS::UserRole::User);
	ftp.addUser("admin", "1234", IFS::UserRole::Admin);
	// You can also use special FTP command: "fsformat" for clearing file system (for example from TotalCMD)
}

// Will be called when WiFi station timeout was reached
void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial.println(_F("I'm NOT CONNECTED. Need help!!! :("));
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	/*
	 * Mount file system, in order to work with files.
	 *
	 * This sample uses an FWFS (Firmware FileSystem) partition to store
	 * files which don't change. This considerably reduces the amount of data
	 * which might be erased in the event of write failure, etc.
	 */

	// This option mounts just the SPIFFS partition
	// spiffs_mount();

	// Mount only the FWFS partition
	// fwfs_mount();

	/*
	 * Use the 'Hybrid' filesystem, with SPIFFS layered over FWFS.
	 * 
	 * When a file is opened for writing it is transparently copied to the SPIFFS partition so it can be updated.
	 * Wiping the SPIFFS partition reverts the filesystem to its original state.
	 *
	 * Note that files marked as ‘read-only’ may not be written in this manner.
	 */
	hyfs_mount();

	fileSetContent("example.txt", "hello world!");
	fileSetContent("data.bin", "\1\2\3\4\5");

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
