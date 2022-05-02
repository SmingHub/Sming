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

#include <Wire.h>
#include <Hosted/Client.h>

extern Hosted::Client* hostedClient;

void TwoWire::begin(uint8_t sda, uint8_t scl)
{
	hostedClient->send(__func__, sda, scl);
}

void TwoWire::pins(uint8_t sda, uint8_t scl)
{
	hostedClient->send(__func__, sda, scl);
}

void TwoWire::begin()
{
	hostedClient->send(__func__);
}

void TwoWire::end()
{
	hostedClient->send(__func__);
}

TwoWire::Status TwoWire::status()
{
	hostedClient->send(__func__);
	return hostedClient->wait<TwoWire::Status>();
}

void TwoWire::setClock(uint32_t freq)
{
	hostedClient->send(__func__, freq);
}

void TwoWire::setClockStretchLimit(uint32_t limit)
{
	hostedClient->send(__func__, limit);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t size, bool sendStop)
{
	hostedClient->send(__func__, address, size, sendStop);
	return hostedClient->wait<uint8_t>();
}

void TwoWire::beginTransmission(uint8_t address)
{
	hostedClient->send(__func__, address);
}

TwoWire::Error TwoWire::endTransmission(bool sendStop)
{
	hostedClient->send(__func__, sendStop);
	return hostedClient->wait<TwoWire::Error>();
}

size_t TwoWire::write(uint8_t data)
{
	hostedClient->send(__func__, data);
	return hostedClient->wait<size_t>();
}

size_t TwoWire::write(const uint8_t* data, size_t quantity)
{
	hostedClient->send(__func__, data, quantity);
	return hostedClient->wait<size_t>();
}

int TwoWire::available()
{
	hostedClient->send(__func__);
	return hostedClient->wait<int>();
}

int TwoWire::read()
{
	hostedClient->send(__func__);
	return hostedClient->wait<int>();
}

int TwoWire::peek()
{
	hostedClient->send(__func__);
	return hostedClient->wait<int>();
}

void TwoWire::flush()
{
	hostedClient->send(__func__);
}

void TwoWire::onReceiveService(uint8_t* inBytes, int numBytes)
{
	// don't bother if user hasn't registered a callback
	// if(!userReceiveCallback){
	//   return;
	// }
	// // don't bother if rx buffer is in use by a master requestFrom() op
	// // i know this drops data, but it allows for slight stupidity
	// // meaning, they may not have read all the master requestFrom() data yet
	// if(rxBufferIndex < rxBufferLength){
	//   return;
	// }
	// // copy twi rx buffer into local read buffer
	// // this enables new reads to happen in parallel
	// for(uint8_t i = 0; i < numBytes; ++i){
	//   rxBuffer[i] = inBytes[i];
	// }
	// // set rx iterator vars
	// rxBufferIndex = 0;
	// rxBufferLength = numBytes;
	// // alert user program
	// user_onReceive(numBytes);
}

void TwoWire::onRequestService()
{
	// // don't bother if user hasn't registered a callback
	// if(!userRequestCallback){
	//   return;
	// }
	// // reset tx buffer iterator vars
	// // !!! this will kill any pending pre-master sendTo() activity
	// txBufferIndex = 0;
	// txBufferLength = 0;
	// // alert user program
	// user_onRequest();
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWIRE)
TwoWire Wire;
#endif

