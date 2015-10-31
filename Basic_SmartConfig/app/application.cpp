#include <user_config.h>
#include <SmingCore/SmingCore.h>

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	WifiStation.smartConfigStart(SCT_EspTouch);
}
