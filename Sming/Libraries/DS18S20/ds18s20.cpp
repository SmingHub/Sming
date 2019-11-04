/*
 * ds18s20.cpp
 *
 *  Created on: 01-09-2015
 *      Author: flexiti and Anakod
 */

#include "Arduino.h"
#include <Libraries/OneWire/OneWire.h>
#include "ds18s20.h"

#define DEBUG_DS18S20


#ifdef DEBUG_DS18S20
  #define debugx(fmt, ...) debugf(fmt, ##__VA_ARGS__)
#else
 #define debugx(fmt, ...)
#endif








DS18S20::DS18S20(uint8_t workPin /* = DS1820_WORK_PIN */)
{
	ds = new OneWire(workPin);
}

DS18S20::~DS18S20()
{
	delete ds;
	ds = nullptr;
}

void DS18S20::Init(uint8_t pinOneWire)
{
   ds->begin(pinOneWire);
   InProgress=false;
}

void DS18S20::StartMeasure()
{
	if (!InProgress)
	{

		debugx("  DBG: DS1820 reading task start, try to read up to %d sensors",MAX_SENSORS);
		InProgress=true;
		ds->begin();
		ds->reset_search();
		DelaysTimer.initializeMs(150, TimerDelegate(&DS18S20::DoSearch, this)).start(false);
	}

}



uint8_t DS18S20::FindAlladdresses()
{
	uint8_t counter=0;
	uint8_t thermoNo=0;
	uint8_t a;

	while (counter++ < MAX_SENSORS)
	{
		if (!ds->search(addr))
		{
			debugx("  DBG: No more address found");
            break;
		}
		else
		if (OneWire::crc8(addr, 7) == addr[7])
			{
			  debugx("  DBG: CRC is valid");
				switch (addr[0]) {
				case 0x10:
				  debugx("  DBG: Chip = DS18S20");  // or old DS1820
				  type_s[thermoNo] = 1;
				  break;
				case 0x28:
				  debugx("  DBG: Chip = DS18B20");
				  type_s[thermoNo] = 0;
				  break;
				case 0x22:
				  debugx("  DBG: Chip = DS1822");
				  type_s[thermoNo] = 0;
				  break;
				default:
				  debugx("  DBG: This Device is not a DS18x20 family device.");
				  break;
				}

			    // address ok, copy uwaga na default teraz zle
				for(a=0;a<8;a++)
				addresses[thermoNo]=(addresses[thermoNo]<<8) +(uint64_t)addr[a];
                thermoNo++;
			}
			else
			{
			debugx("  DBG: CRC is not valid");
			}
	}


  return thermoNo;
}

void DS18S20::DoSearch(void)
{

	numberOf=FindAlladdresses();

	if (!numberOf)
	{
		debugx("  DBG: No DS1820 sensor found");
        InProgress=false;

	}
	else
	{
      debugx("  DBG: %d DS1820 sensors found",numberOf);

      numberOfread=0;
      StartReadNext();
	}


}

void DS18S20::StartReadNext()
{
   if (numberOf > numberOfread )
   {
	ds->reset();

	  uint64_t tmp=addresses[numberOfread];
	  for (uint8_t a=0;a<8;a++)
	  {
	    addr[7-a]=(uint8_t)tmp;
	    tmp=tmp>>8;
	  }

	  ds->select(addr);
	  ds->write(STARTCONVO, 1);        // start conversion, with parasite power on at the end

	  DelaysTimer.initializeMs(900, TimerDelegate(&DS18S20::DoMeasure, this)).start(false);
   }
   else
   {
	   debugx("  DBG: DS18S20 reading task end");
	   InProgress=false;
	   if(readEndCallback) //If callback set, execute function
	   {
		   readEndCallback();
	   }
   }
}

void DS18S20::DoMeasure()
{

	uint8_t present,i;
	present = ds->reset();
	ds->select(addr);
	ds->write(READSCRATCH);         // Read Scratchpad

	debugx("  DBG: T%d",numberOfread+1);

	for ( i = 0; i < 9; i++)
	{
		// we need 9 bytes
		data[i] = ds->read();
	}
	debugx("  DBG: Data = %x %x %x %x %x %x %x %x %x %x  CRC=%x",present,data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],OneWire::crc8(data, 8));

	// Convert the data to actual temperature
	// because the result is a 16 bit signed integer, it should
	// be stored to an "int16_t" type, which is always 16 bits
	// even when compiled on a 32 bit processor.
	unsigned int raw = (data[1] << 8) | data[0];
	if (type_s[numberOfread])
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

	ValidTemperature[numberOfread]=true;
	
    if (raw & 0x8000)   //is minus ?
	  celsius[numberOfread] = 0 - ((float) ((raw ^ 0xffff) + 1) / 16.0); // 2's comp
	else
	  celsius[numberOfread] = (float)raw / 16.0;	

	fahrenheit[numberOfread] = celsius[numberOfread] * 1.8 + 32.0;

	debugx("  DBG: Temperature = %f Celsius, %f Fahrenheit",celsius[numberOfread],fahrenheit[numberOfread]);

	numberOfread++;
	DelaysTimer.initializeMs(100, TimerDelegate(&DS18S20::StartReadNext, this)).start(false);

}

float DS18S20::GetCelsius(uint8_t index)
{
	  if (index < numberOf)
	     return celsius[index];
	  else
		 return 0;

}

float DS18S20::GetFahrenheit(uint8_t index)
{
	  if (index <= numberOf)
	     return fahrenheit[index];
	  else
		 return 0;

}


bool DS18S20::IsValidTemperature(uint8_t index)
{
	  if (index <= numberOf)
		  return ValidTemperature[index];
	  else
		  return false;
}

bool DS18S20::MeasureStatus()
{
	return InProgress;
}

uint64_t DS18S20::GetSensorID(uint8_t index)
{
	  if (index <= numberOf)
		  return addresses[index];
	  else
		  return 0;
}

uint8_t DS18S20::GetSensorsCount()
{
		  return numberOf;
}

void DS18S20::RegisterEndCallback(DS18S20CompletedDelegate callback)
{
	readEndCallback = callback;
}

void DS18S20::UnRegisterCallback()
{
	readEndCallback = nullptr;
}

