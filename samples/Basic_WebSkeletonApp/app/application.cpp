#include <SmingCore.h>
#include <webserver.h>
#include <configuration.h>

// Global
unsigned long counter;

namespace
{
SimpleTimer counterTimer;

void counterCallback()
{
	counter++;
}

void STADisconnect(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial << _F("DISCONNECT - SSID: ") << ssid << _F(", REASON: ") << WifiEvents.getDisconnectReasonDesc(reason)
		   << endl;

	if(!WifiAccessPoint.isEnabled()) {
		Serial << _F("Starting OWN AP");
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
		WifiStation.connect();
	}
}

void STAGotIP(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial << _F("GOTIP - IP: ") << ip << _F(", MASK: ") << mask << _F(", GW: ") << gateway << endl;

	if(WifiAccessPoint.isEnabled()) {
		debugf("Shutdown OWN AP");
		WifiAccessPoint.enable(false);
	}
	// Add commands to be executed after successfully connecting to AP and got IP from it
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);

#ifndef ENABLE_FLASHSTRING_MAP
	spiffs_mount(); // Mount file system, in order to work with files
#endif

	// Set higher CPU freq & disable wifi sleep
	// System.setCpuFrequency(CpuCycleClockFast::cpuFrequency());
	wifi_set_sleep_type(NONE_SLEEP_T);

	activeConfig = loadConfig();
	if(activeConfig.StaSSID) {
		WifiStation.enable(true);
		WifiStation.config(activeConfig.StaSSID, activeConfig.StaPassword);
	}

	// Attach Wifi events handlers
	WifiEvents.onStationDisconnect(STADisconnect);
	WifiEvents.onStationGotIP(STAGotIP);

	System.onReady(startWebServer);

	counterTimer.initializeMs<1000>(counterCallback).start();
}
