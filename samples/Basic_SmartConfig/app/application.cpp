#include <SmingCore.h>

bool smartConfigCallback(SmartConfigEvent event, const SmartConfigEventInfo& info)
{
	switch(event) {
	case SCE_Wait:
		debugf("SCE_Wait\n");
		break;
	case SCE_FindChannel:
		debugf("SCE_FindChannel\n");
		break;
	case SCE_GettingSsid:
		debugf("SCE_GettingSsid, type = %d\n", info.type);
		break;
	case SCE_Link:
		debugf("SCE_Link\n");
		WifiStation.config(info.ssid, info.password);
		WifiStation.connect();
		break;
	case SCE_LinkOver:
		debugf("SCE_LinkOver\n");
		WifiStation.smartConfigStop();
		break;
	}

	// Don't do any internal processing
	return false;
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	// automatic (acts as the sample callback above)
	//WifiStation.smartConfigStart(SCT_EspTouch);
	// manual, use callback above
	WifiStation.smartConfigStart(SCT_EspTouch, smartConfigCallback);
}
