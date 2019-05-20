#include <SmingCore.h>

void smartConfigCallback(sc_status status, void* pdata)
{
	switch(status) {
	case SC_STATUS_WAIT:
		debugf("SC_STATUS_WAIT\n");
		break;
	case SC_STATUS_FIND_CHANNEL:
		debugf("SC_STATUS_FIND_CHANNEL\n");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		debugf("SC_STATUS_GETTING_SSID_PSWD\n");
		break;
	case SC_STATUS_LINK: {
		debugf("SC_STATUS_LINK\n");
		station_config* sta_conf = (station_config*)pdata;
		char* ssid = (char*)sta_conf->ssid;
		char* password = (char*)sta_conf->password;
		WifiStation.config(ssid, password);
		WifiStation.connect();
	} break;
	case SC_STATUS_LINK_OVER:
		debugf("SC_STATUS_LINK_OVER\n");
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
