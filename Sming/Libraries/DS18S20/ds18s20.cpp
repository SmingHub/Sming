/*
 * ds18s20.cpp
 *
 *  Created on: 01-09-2015
 *      Author: mariuszb
 */

#include "Arduino.h"
#include "OneWire.h"
#include "ds18s20.h"



#define WORK_PIN 2 			// default DS1820 on GPIO2, can be changed by Init
OneWire ds(WORK_PIN);


DS18S20::DS18S20()
{
}

DS18S20::~DS18S20()
{
}

void DS18S20::Init(uint8_t pinOneWire)
{
   ds.begin(pinOneWire);
}

void DS18S20::StartMeasure(void)
{

	if (!InProgress)
	{
		debugf("  DS1820 reading task start");
		InProgress=true;
		ds.begin();
		ds.reset_search();
		DelaysTimer.initializeMs(150, TimerDelegate(&DS18S20::DoSearch, this)).start(false);
	}

}



void DS18S20::DoSearch(void)
{
	if (!ds.search(addr))
	{
		debugf("  No addresses found.");
        ValidTemperature=false;
        InProgress=false;

	}
	else
	{

	if (OneWire::crc8(addr, 7) == addr[7])
	{
	  debugf("  DS1820 CRC is valid!");

		switch (addr[0]) {
		case 0x10:
		  debugf("  Chip = DS18S20");  // or old DS1820
		  type_s = 1;
		  break;
		case 0x28:
		  debugf("  Chip = DS18B20");
		  type_s = 0;
		  break;
		case 0x22:
		  debugf("  Chip = DS1822");
		  type_s = 0;
		  break;
		default:
		  debugf("  Device is not a DS18x20 family device.");
		  ValidTemperature=false;
		  InProgress=false;
		  return;
		}

	  ds.reset();
	  ds.select(addr);
	  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

	  DelaysTimer.initializeMs(1000, TimerDelegate(&DS18S20::DoMeasure, this)).start(false);

	}
	else
	{
	debugf("  Thermometer ROM CRC ERROR");
    ValidTemperature=false;
    InProgress=false;
	}
	}
}



void DS18S20::DoMeasure()
{

	uint8_t present,i;
	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);         // Read Scratchpad


	for ( i = 0; i < 9; i++)
	{
		// we need 9 bytes
		data[i] = ds.read();
	}
	debugf("  Data = %x %x %x %x %x %x %x %x %x %x  CRC=%x",present,data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],OneWire::crc8(data, 8));

	// Convert the data to actual temperature
	// because the result is a 16 bit signed integer, it should
	// be stored to an "int16_t" type, which is always 16 bits
	// even when compiled on a 32 bit processor.
	unsigned int raw = (data[1] << 8) | data[0];
	if (type_s)
	{
		raw = raw << 3; // 9 bit resolution default
		if (data[7] == 0x10)
		{
		  // "count remain" gives full 12 bit resolution
		  raw = (raw & 0xFFF0) + 12 - data[6];
		}
	} else {
		byte cfg = (data[4] & 0x60);
		// at lower res, the low bits are undefined, so let's zero them
		if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
		else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
		else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
		//// default is 12 bit resolution, 750 ms conversion time
	}

	celsius = (float)raw / 16.0;
	fahrenheit = celsius * 1.8 + 32.0;

	debugf("  Temperature = %f Celsius, %f Fahrenheit",celsius,fahrenheit);
	debugf("  DS18S20 reading task end");

	ValidTemperature=true;
	InProgress=false;

}

float DS18S20::GetCelsius()
{
	return celsius;
}

float DS18S20::GetFahrenheit()
{
	return fahrenheit;
}


bool DS18S20::IsValidTemperature()
{
	return ValidTemperature;
}

bool DS18S20::Status()
{
	return InProgress;
}

