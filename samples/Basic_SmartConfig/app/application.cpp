#include <SmingCore.h>

void smartConfigCallback(SmartConfigEvent status, void* pdata)
{
	switch(status) {
	case SCE_Wait:
		debugf("SCE_Wait\n");
		break;
	case SCE_FindChannel:
		debugf("SCE_FindChannel\n");
		break;
	case SCE_GotSsid:
		debugf("SCE_GotSsid\n");
		break;
	case SCE_Link: {
		debugf("SCE_Link\n");
		station_config* sta_conf = (station_config*)pdata;
		char* ssid = (char*)sta_conf->ssid;
		char* password = (char*)sta_conf->password;
		WifiStation.config(ssid, password);
		WifiStation.connect();
	} break;
	case SCE_LinkOver:
		debugf("SCE_LinkOver\n");
		WifiStation.smartConfigStop();
		break;
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	// autmoatic (acts as the sample callback above)
	//WifiStation.smartConfigStart(SCT_EspTouch);
	// manual, use callback above
	WifiStation.smartConfigStart(SCT_EspTouch, smartConfigCallback);
}
