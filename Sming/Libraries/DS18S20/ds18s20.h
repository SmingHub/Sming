/*
 * ds18s20.h
 *
 *  Created on: 01-09-2015
 *      Author: mariuszb
 */

#ifndef READFROMDS_H_
#define READFROMDS_H_

#include "../../SmingCore/Wire.h"


class DS18S20
{
public:
	DS18S20();
	void Init(uint8_t);
	void StartMeasure();
	float GetCelsius();
	float GetFahrenheit();
	bool IsValidTemperature();
	bool Status();

	virtual ~DS18S20();

private:

	void DoMesure();
	void DoSearch();

private:
	bool InProgress = false;
	bool ValidTemperature = false;
	uint8_t addr[8];
	uint8_t type_s=0;
	uint8_t data[12];


	Timer DelaysTimer;

	float celsius, fahrenheit;



};

#endif /* READFROMDS_H_ */
