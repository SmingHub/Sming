#include <SmingCore.h>

namespace
{
bool smartConfigCallback(SmartConfigEvent event, const SmartConfigEventInfo& info)
{
	switch(event) {
	case SCE_Wait:
		Serial.println(_F("SCE_Wait"));
		break;

	case SCE_FindChannel:
		Serial.println(_F("SCE_FindChannel"));
		break;

	case SCE_GettingSsid:
		Serial << _F("SCE_GettingSsid, type = ") << info.type << endl;
		break;

	case SCE_Link:
		Serial.println(_F("SCE_Link"));
		WifiStation.config(info.ssid, info.password);
		WifiStation.connect();
		break;

	case SCE_LinkOver:
		Serial.println(_F("SCE_LinkOver"));
		WifiStation.smartConfigStop();
		break;
	}

	// Don't do any internal processing
	return false;
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial << _F("Connected: ") << ip << endl;
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiEvents.onStationGotIP(gotIP);
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);

	// automatic (acts as the sample callback above)
	// WifiStation.smartConfigStart(SCT_EspTouch);
	// manual, use callback above
	WifiStation.smartConfigStart(SCT_EspTouch, smartConfigCallback);
}
