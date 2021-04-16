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

Hosted::Client* hostedClient{nullptr};

extern void init();

void host_init()
{
	Serial.begin(115200);
	hostedClient = new Hosted::Client(Serial);
	hostedClient->getRemoteCommands();
	init();
}
