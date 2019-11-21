#include <tytherm.h>

static Timer counterTimer;
unsigned long counter = 0;

static void counterLoop()
{
	counter++;
}

static void STADisconnect(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("DISCONNECT - SSID: %s, REASON: %s\n", ssid.c_str(), WifiEvents.getDisconnectReasonDesc(reason).c_str());

	if(!WifiAccessPoint.isEnabled()) {
		debugf("Starting OWN AP");
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
		WifiStation.connect();
	}
}

static void STAGotIP(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	debugf("GOTIP - IP: %s, MASK: %s, GW: %s\n", ip.toString().c_str(), mask.toString().c_str(),
		   gateway.toString().c_str());

	if(WifiAccessPoint.isEnabled()) {
		debugf("Shutdown OWN AP");
		WifiAccessPoint.enable(false);
	}
	// Add commands to be executed after successfully connecting to AP and got IP from it
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);
	Serial.commandProcessing(false);

	spiffs_mount(); // Mount file system, in order to work with files

	//SET higher CPU freq & disable wifi sleep
	//	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	activeConfig = loadConfig();
	if(activeConfig.StaSSID) {
		WifiStation.enable(true);
		WifiStation.config(activeConfig.StaSSID, activeConfig.StaPassword);
	}

	// Attach Wifi events handlers
	WifiEvents.onStationDisconnect(STADisconnect);
	WifiEvents.onStationGotIP(STAGotIP);

	startWebServer();

	counterTimer.initializeMs(1000, counterLoop).start();
}
