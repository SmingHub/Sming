/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Wire.cpp
 *
 * @author 2022 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#if ENABLE_HOSTED_WIRE

#include <Wire.h>
#include <Hosted/Client.h>
#include <Hosted/Util.h>

extern Hosted::Client* hostedClient;

void TwoWire::begin(uint8_t sda, uint8_t scl)
{
	hostedClient->send(__PRETTY_FUNCTION__, sda, scl);
}

void TwoWire::pins(uint8_t sda, uint8_t scl)
{
	hostedClient->send(__PRETTY_FUNCTION__, sda, scl);
}

void TwoWire::begin()
{
	hostedClient->send(__PRETTY_FUNCTION__);
}

void TwoWire::end()
{
	hostedClient->send(__PRETTY_FUNCTION__);
}

TwoWire::Status TwoWire::status()
{
	hostedClient->send(__PRETTY_FUNCTION__);
	return hostedClient->wait<TwoWire::Status>();
}

void TwoWire::setClock(uint32_t freq)
{
	hostedClient->send(__PRETTY_FUNCTION__, freq);
}

void TwoWire::setClockStretchLimit(uint32_t limit)
{
	hostedClient->send(__PRETTY_FUNCTION__, limit);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t size, bool sendStop)
{
	hostedClient->send(__PRETTY_FUNCTION__, address, size, sendStop);
	return hostedClient->wait<uint8_t>();
}

void TwoWire::beginTransmission(uint8_t address)
{
	hostedClient->send(__PRETTY_FUNCTION__, address);
}

TwoWire::Error TwoWire::endTransmission(bool sendStop)
{
	hostedClient->send(__PRETTY_FUNCTION__, sendStop);
	return hostedClient->wait<TwoWire::Error>();
}

size_t TwoWire::write(uint8_t data)
{
	hostedClient->send(__PRETTY_FUNCTION__, data);
	return hostedClient->wait<size_t>();
}

size_t TwoWire::write(const uint8_t* data, size_t quantity)
{
	hostedClient->send(__PRETTY_FUNCTION__, data, quantity);
	return hostedClient->wait<size_t>();
}

int TwoWire::available()
{
	hostedClient->send(__PRETTY_FUNCTION__);
	return hostedClient->wait<int>();
}

int TwoWire::read()
{
	hostedClient->send(__PRETTY_FUNCTION__);
	return hostedClient->wait<int>();
}

int TwoWire::peek()
{
	hostedClient->send(__PRETTY_FUNCTION__);
	return hostedClient->wait<int>();
}

void TwoWire::flush()
{
	hostedClient->send(__PRETTY_FUNCTION__);
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWIRE)
TwoWire Wire;
#endif

#endif /* ENABLE_HOSTED_WIRE */
