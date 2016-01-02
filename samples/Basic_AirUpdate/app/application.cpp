#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

#define UPDATE_PIN 0 // GPIO0
HttpFirmwareUpdate airUpdater;

void IRAM_ATTR interruptHandler()
{
	detachInterrupt(UPDATE_PIN);
	Serial.println("Let's do cloud magic!");

	// Start cloud update
	airUpdater.start();
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	// Configure cloud update
	airUpdater.addItem(0x0000, "http://simple.anakod.ru/fw/eagle.flash.bin");
	airUpdater.addItem(0x9000, "http://simple.anakod.ru/fw/eagle.irom0text.bin");

	attachInterrupt(UPDATE_PIN, interruptHandler, CHANGE);
	Serial.println("\r\nPress GPIO0 to start cloud update!\r\n");
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);
}
