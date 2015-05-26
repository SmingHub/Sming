#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "smartconfig.h"

void smartconfigDone(sc_status status, void *pdata)
{
	//@TODO: add here the smart config status processing
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.enable(true);
	smartconfig_start(SC_TYPE_ESPTOUCH, smartconfigDone);
}
