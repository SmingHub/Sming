/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Digital.cpp
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#include <Digital.h>
#include <Hosted/Client.h>

extern Hosted::Client* hostedClient;

void pinMode(uint16_t pin, uint8_t mode)
{
	hostedClient->send(__func__, pin, mode);
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	hostedClient->send(__func__, pin, val);
}

uint8_t digitalRead(uint16_t pin)
{
	hostedClient->send(__func__, pin);
	return hostedClient->wait<uint8_t>();
}

unsigned long pulseIn(uint16_t pin, uint8_t state, unsigned long timeout)
{
	hostedClient->send(__func__, pin, state, timeout);
	return hostedClient->wait<unsigned long>();
}
