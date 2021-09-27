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

#include <SmingCore.h>
#include <Hosted/Client.h>
#include <Hosted/Serial.h>

#ifndef HOSTED_COM_PORT
#define HOSTED_COM_PORT "/dev/ttyUSB0"
#endif

#ifndef HOSTED_COM_SPEED
#define HOSTED_COM_SPEED 115200
#endif

Hosted::Client* hostedClient{nullptr};

extern void init();

extern "C" {
void __real_host_init();
void __wrap_host_init();
}

Hosted::Serial hostedSerial(HOSTED_COM_PORT);

void __wrap_host_init()
{
	Serial.begin(115200);
	Serial.printf("Connecting to: %s ...\n", HOSTED_COM_PORT);

	bool serialReady = false;
	do {
		serialReady = hostedSerial.begin(HOSTED_COM_SPEED);
		usleep(200);
	} while(!serialReady);

	hostedClient = new Hosted::Client(hostedSerial);
	hostedClient->getRemoteCommands();
	init();
}
