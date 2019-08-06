#include <tytherm.h>

Timer counterTimer;
void counterLoop();
unsigned long counter = 0;

void STADisconnect(const String& ssid, const MACAddress& bssid, WifiDisconnectReason reason);
void STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway);

void init()
{
	spiffs_mount();					// Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false);
	Serial.commandProcessing(false);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	ActiveConfig = loadConfig();

	// Attach Wifi events handlers
	WifiEvents.onStationDisconnect(STADisconnect);
	WifiEvents.onStationGotIP(STAGotIP);

	startWebServer();

	counterTimer.initializeMs(1000, counterLoop).start();
}

void counterLoop()
{
	counter++;
}

void STADisconnect(const String& ssid, const MACAddress& bssid, WifiDisconnectReason reason)
{
	debugf("DISCONNECT - SSID: %s, REASON: %d\n", ssid.c_str(), reason);

	if(!WifiAccessPoint.isEnabled()) {
		debugf("Starting OWN AP");
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
		WifiStation.connect();
	}
}

void STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway)
{
	debugf("GOTIP - IP: %s, MASK: %s, GW: %s\n", ip.toString().c_str(), mask.toString().c_str(),
		   gateway.toString().c_str());

	if(WifiAccessPoint.isEnabled()) {
		debugf("Shutdown OWN AP");
		WifiAccessPoint.enable(false);
	}
	// Add commands to be executed after successfully connecting to AP and got IP from it
}
