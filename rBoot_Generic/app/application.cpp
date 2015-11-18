#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/RBootClass.h>
#include <SmingCore/Network/TelnetServer.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

RBoot rBoot;
TelnetServer telnetServer;

void rbootDelegate(int resultCode) {

	Serial.printf("rBoot resultcode =  %d\r\n", resultCode );
}

void addSpiffsRoms()
{
	rboot_config bootconf;
	bootconf = rboot_get_config();
	bootconf.count = 4;
	bootconf.roms[2] = 0x100000;
	bootconf.roms[3] = 0x300000;
	rboot_set_config(&bootconf);
}

void init() {
	
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);
	
	telnetServer.listen(23);
	telnetServer.setDebug(true);

	Serial.commandProcessing(true);

	addSpiffsRoms();

	rBoot.initCommand();
	rBoot.setDelegate(rbootDelegate);
	int slot = rBoot.getCurrentRom();
	
	Serial.printf("\r\nrBoot Generic running rom %d.\r\n", slot);

	rBoot.mountSpiffs(2);
}
