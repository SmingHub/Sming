/*
  TwoWire.cpp - TWI/I2C library for Arduino & Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  si2c.c - Software I2C library for esp8266
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified December 2014 by Ivan Grokhotkov (ivan@esp8266.com) - esp8266 support
  Modified April 2015 by Hrsto Gochkov (ficeto@ficeto.com) - alternative esp8266 support
*/

#include "Wire.h"
#include "Digital.h"
#include <twi_arch.h>
#include <Platform/System.h>

#if defined(ARCH_ESP8266) | defined(ARCH_HOST)
#define DEFAULT_SDA_PIN 2
#define DEFAULT_SCL_PIN 0
#elif defined(ARCH_ESP32)
#define DEFAULT_SDA_PIN 21
#define DEFAULT_SCL_PIN 22
#elif defined(ARCH_RP2040)
#include <pico.h>
#define DEFAULT_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN
#define DEFAULT_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN
#endif

#ifndef FCPU80
#define FCPU80 80000000L
#endif

#if F_CPU == FCPU80
#define TWI_CLOCK_STRETCH_MULTIPLIER 3
#else
#define TWI_CLOCK_STRETCH_MULTIPLIER 6
#endif

void TwoWire::begin(uint8_t sda, uint8_t scl)
{
	debug_d("[TWI] begin(%u, %u)", sda, scl);

	twi_sda = sda;
	twi_scl = scl;
	pinMode(twi_sda, INPUT_PULLUP);
	pinMode(twi_scl, INPUT_PULLUP);
	setClock(100000);
	setClockStretchLimit(230); // default value is 230 uS

	flush();
}

void TwoWire::pins(uint8_t sda, uint8_t scl)
{
	debug_d("[TWI] pins(%u, %u)", sda, scl);

	twi_sda = sda;
	twi_scl = scl;
}

void TwoWire::begin()
{
	begin(DEFAULT_SDA_PIN, DEFAULT_SCL_PIN);
}

void TwoWire::end()
{
	debug_d("[TWI] end(%u, %u)", twi_sda, twi_scl);
	pinMode(twi_sda, INPUT);
	pinMode(twi_scl, INPUT);
}

TwoWire::Status TwoWire::status()
{
	if(SCL_READ() == 0) {
		// SCL held low by another device, no procedure available to recover
		return I2C_SCL_HELD_LOW;
	}

	int clockCount = 20;

	// if SDA low, read the bits slaves have to sent to a max
	while(SDA_READ() == 0 && clockCount-- > 0) {
		twi_read_bit();
		if(SCL_READ() == 0) {
			// SCL held low beyond slave clock stretch time
			return I2C_SCL_HELD_LOW_AFTER_READ;
		}
	}

	if(SDA_READ() == 0) {
		// SDA line held low by slave/another_master after n bits
		return I2C_SDA_HELD_LOW;
	}

	if(!twi_write_start()) {
		// Line busy. SDA again held low by another device. 2nd master?
		return I2C_SDA_HELD_LOW_AFTER_INIT;
	}

	return I2C_OK;
}

void TwoWire::setClock(uint32_t freq)
{
	auto sys = System.getCpuFrequency();
	if(sys == eCF_80MHz) {
		if(freq <= 100000) {
			twi_dcount = 16; //about 100KHz
		} else if(freq <= 200000) {
			twi_dcount = 5; //about 200KHz
		} else if(freq <= 300000) {
			twi_dcount = 2; //about 300KHz
		} else {
			twi_dcount = 0; //about 400KHz
		}
	} else { // Assume eCF_160MHz
		if(freq <= 100000) {
			twi_dcount = 28; //about 100KHz
		} else if(freq <= 200000) {
			twi_dcount = 12; //about 200KHz
		} else if(freq <= 300000) {
			twi_dcount = 5; //about 300KHz
		} else if(freq <= 400000) {
			twi_dcount = 3; //about 400KHz
		} else if(freq <= 500000) {
			twi_dcount = 1; //about 500KHz
		} else {
			twi_dcount = 0; //about 600KHz
		}
	}
}

void TwoWire::setClockStretchLimit(uint32_t limit)
{
	twi_clockStretchLimit = limit * TWI_CLOCK_STRETCH_MULTIPLIER;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t size, bool sendStop)
{
	if(size == 0) {
		return 0;
	}

	if(size > BUFFER_LENGTH) {
		size = BUFFER_LENGTH;
	}

	auto err = twi_readFrom(address, rxBuffer, size, sendStop);
	rxBufferIndex = 0;
	rxBufferLength = err ? 0 : size;
	return rxBufferLength;
}

void TwoWire::beginTransmission(uint8_t address)
{
	transmitting = true;
	txAddress = address;
	txBufferIndex = 0;
	txBufferLength = 0;
}

TwoWire::Error TwoWire::endTransmission(bool sendStop)
{
	auto err = twi_writeTo(txAddress, txBuffer, txBufferLength, sendStop);
	txBufferIndex = 0;
	txBufferLength = 0;
	transmitting = false;
	return err;
}

size_t TwoWire::write(uint8_t data)
{
	if(transmitting) {
		if(txBufferLength >= BUFFER_LENGTH) {
			setWriteError();
			return 0;
		}
		txBuffer[txBufferIndex] = data;
		++txBufferIndex;
		txBufferLength = txBufferIndex;
	} else {
		// i2c_slave_transmit(&data, 1);
	}
	return 1;
}

size_t TwoWire::write(const uint8_t* data, size_t quantity)
{
	if(transmitting) {
		for(size_t i = 0; i < quantity; ++i) {
			if(write(data[i]) != 1) {
				return i;
			}
		}
	} else {
		// i2c_slave_transmit(data, quantity);
	}
	return quantity;
}

int TwoWire::available()
{
	return rxBufferLength - rxBufferIndex;
}

int TwoWire::read()
{
	int value = -1;
	if(rxBufferIndex < rxBufferLength) {
		value = rxBuffer[rxBufferIndex];
		++rxBufferIndex;
	}
	return value;
}

int TwoWire::peek()
{
	return (rxBufferIndex < rxBufferLength) ? rxBuffer[rxBufferIndex] : -1;
}

void TwoWire::flush()
{
	rxBufferIndex = 0;
	rxBufferLength = 0;
	txBufferIndex = 0;
	txBufferLength = 0;
}

void TwoWire::onReceiveService(uint8_t* inBytes, int numBytes)
{
	// don't bother if user hasn't registered a callback
	// if(!user_onReceive){
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
	// if(!user_onRequest){
	//   return;
	// }
	// // reset tx buffer iterator vars
	// // !!! this will kill any pending pre-master sendTo() activity
	// txBufferIndex = 0;
	// txBufferLength = 0;
	// // alert user program
	// user_onRequest();
}

void TwoWire::twi_delay(uint8_t v)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	for(unsigned i = 0; i < v; i++) {
		(void)SCL_READ();
	}
#pragma GCC diagnostic pop
}

bool TwoWire::twi_write_start()
{
	SCL_HIGH();
	SDA_HIGH();
	if(SDA_READ() == 0) {
		return false;
	}
	twi_delay(twi_dcount);
	SDA_LOW();
	twi_delay(twi_dcount);
	return true;
}

bool TwoWire::twi_write_stop()
{
	SCL_LOW();
	SDA_LOW();
	twi_delay(twi_dcount);
	SCL_HIGH();
	for(unsigned i = 0; SCL_READ() == 0 && i++ < twi_clockStretchLimit;) {
		// Clock stretching
	}
	twi_delay(twi_dcount);
	SDA_HIGH();
	twi_delay(twi_dcount);

	return true;
}

bool TwoWire::twi_write_bit(bool bit)
{
	SCL_LOW();
	if(bit) {
		SDA_HIGH();
	} else {
		SDA_LOW();
	}
	twi_delay(twi_dcount + 1);
	SCL_HIGH();
	for(unsigned i = 0; SCL_READ() == 0 && i++ < twi_clockStretchLimit;) {
		// Clock stretching
	}
	twi_delay(twi_dcount);
	return true;
}

bool TwoWire::twi_read_bit()
{
	SCL_LOW();
	SDA_HIGH();
	twi_delay(twi_dcount + 2);
	SCL_HIGH();
	for(unsigned i = 0; SCL_READ() == 0 && i++ < twi_clockStretchLimit;) {
		// Clock stretching
	}
	bool bit = SDA_READ();
	twi_delay(twi_dcount);
	return bit;
}

bool TwoWire::twi_write_byte(uint8_t byte)
{
	for(uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		twi_write_bit(byte & mask);
	}
	return !twi_read_bit(); //NACK/ACK
}

uint8_t TwoWire::twi_read_byte(bool nack)
{
	uint8_t res{0};
	for(uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		res |= twi_read_bit() ? mask : 0;
	}
	twi_write_bit(nack);
	return res;
}

TwoWire::Error TwoWire::twi_writeTo(uint8_t address, const uint8_t* buf, size_t len, bool sendStop)
{
	if(!twi_write_start()) {
		return I2C_ERR_LINE_BUSY;
	}
	if(!twi_write_byte(((address << 1) | 0) & 0xFF)) {
		if(sendStop) {
			twi_write_stop();
		}
		return I2C_ERR_ADDR_NACK;
	}
	for(unsigned i = 0; i < len; i++) {
		if(!twi_write_byte(buf[i])) {
			if(sendStop) {
				twi_write_stop();
			}
			return I2C_ERR_DATA_NACK;
		}
	}
	if(sendStop) {
		twi_write_stop();
	}
	for(unsigned i = 0; SDA_READ() == 0 && i++ < 10;) {
		SCL_LOW();
		twi_delay(twi_dcount);
		SCL_HIGH();
		twi_delay(twi_dcount);
	}
	return I2C_ERR_SUCCESS;
}

TwoWire::Error TwoWire::twi_readFrom(uint8_t address, uint8_t* buf, size_t len, bool sendStop)
{
	if(!twi_write_start()) {
		return I2C_ERR_LINE_BUSY;
	}
	if(!twi_write_byte(((address << 1) | 1) & 0xFF)) {
		if(sendStop) {
			twi_write_stop();
		}
		return I2C_ERR_ADDR_NACK;
	}
	for(unsigned i = 0; i + 1 < len; i++) {
		buf[i] = twi_read_byte(false);
	}
	buf[len - 1] = twi_read_byte(true);
	if(sendStop) {
		twi_write_stop();
	}
	for(unsigned i = 0; SDA_READ() == 0 && i++ < 10;) {
		SCL_LOW();
		twi_delay(twi_dcount);
		SCL_HIGH();
		twi_delay(twi_dcount);
	}
	return I2C_ERR_SUCCESS;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWIRE)
TwoWire Wire;
#endif
