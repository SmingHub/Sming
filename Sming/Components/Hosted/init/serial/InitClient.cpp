/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * InitClient.cpp
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#include <Hosted/Serial.h>
#include <Hosted/Client.h>

#ifndef HOSTED_COM_PORT
#define HOSTED_COM_PORT "/dev/ttyUSB0"
#endif

#ifndef HOSTED_COM_SPEED
#define HOSTED_COM_SPEED 115200
#endif

Hosted::Client* hostedClient;

extern "C" void __real__Z4initv();

namespace
{
Hosted::Serial hostedSerial(HOSTED_COM_PORT);
}

extern "C" void __wrap__Z4initv()
{
	host_printf("Connecting to: %s ...\r\n", HOSTED_COM_PORT);

	while(!hostedSerial.begin(HOSTED_COM_SPEED)) {
		msleep(50);
	}

	hostedClient = new Hosted::Client(hostedSerial, '>');
	hostedClient->getRemoteCommands();

	__real__Z4initv();
}
