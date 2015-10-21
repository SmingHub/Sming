/*
 * ds18s20.h
 *
 *  Created on: 01-09-2015
 *      Author: flexiti and Anakod
 */

#ifndef READFROMDS_H_
#define READFROMDS_H_

#include "../../SmingCore/Wire.h"

#define MAX_SENSORS 4         //how many sensors reads up max

// OneWire commands

#define COPYSCRATCH     0x48  // Copy EEPROM
#define READSCRATCH     0xBE  // Read EEPROM
#define WRITESCRATCH    0x4E  // Write EEPROM
#define RECALLSCRATCH   0xB8  // Reload last
#define READPOWERSUPPLY 0xB4  // parasite power
#define ALARMSEARCH     0xEC  // Query for alarm
#define STARTCONVO      0x44  // temperature reading


class DS18S20
{
public:
	DS18S20();
	void Init(uint8_t);			//call for OneWire init
	void StartMeasure();        //Start measurement result after 1.2 seconds * number of sensors
	float GetCelsius(uint8_t);
	float GetFahrenheit(uint8_t);
	bool IsValidTemperature(uint8_t);
	bool MeasureStatus();
	uint64_t GetSensorID(uint8_t);
	uint8_t GetSensorsCount();				//how many the sensors detected
	virtual ~DS18S20();

private:

	void DoMeasure();
	void DoSearch();
	void StartReadNext();
	uint8_t FindAlladdresses();

private:
	bool InProgress = false;
	bool ValidTemperature[MAX_SENSORS];
	uint8_t addr[8];
	uint8_t type_s[MAX_SENSORS];
	uint8_t data[12];
	uint64_t addresses[MAX_SENSORS];
	uint8_t numberOf=0;
	uint8_t numberOfread=0;


	Timer DelaysTimer;


	float celsius[MAX_SENSORS], fahrenheit[MAX_SENSORS];



};

#endif /* READFROMDS_H_ */

