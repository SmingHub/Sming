/*
 * Sming library: Ultrasonic sonar modules driver
 *
 * Tested on modules:
 *   HC-SR04 - ranges: 2-400cm, power: 5v, levels: TTL, for work with 3.3v need voltage divider on ECHO pin
 *   US-100  - power: 3.3v-5v, temp. compensation
 *
 *    Author: sharky <nik.sharky@gmail.com>
 * Thanks to: ITead studio, DIMOSUS, ElecFreaks, Wiki, Parallax for information and ideas
 *
 * Changed: 2015-04-07
 *    TODO: Add multisampling to ping and range methods
 *    TODO: Add setMedium method to set current medium parameters
 *    TODO: Fix bugs with sqrt in temp measurement
 */

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class Ultrasonic
{
public:
	Ultrasonic();
	void init(uint16_t trigPin, uint16_t echoPin);
	uint32_t ping();
	uint16_t us2cm(unsigned long microseconds);
	uint16_t us2inch(unsigned long microseconds);
	uint16_t rangeCM();
	uint16_t rangeInch();
	uint16_t temp(uint16_t baseDist, uint16_t baseTemp, uint16_t samples);

private:
	uint16_t pinTRIG;
	uint16_t pinECHO;

	// Duration of trig pulse (microseconds)
	uint16_t trigDuration = 10;

	// Timeout for receiving echo after trig pulse (microseconds)
	// 50ms by default
	uint16_t echoTimeout = 50000;

	// X - adiabatic index of medium (is about 1.4 for air under normal conditions of pressure and temperature)
	// R - gas constant for medium (for air approximately 8.3145 J/mol·K, ~286.9 J/kg·K)
	// See http://www.engineeringtoolbox.com/individual-universal-gas-constant-d_588.html for details
	float X = 1.4;
	float R = 286.9;
	float XR = X * R;

	// Kelvin/Celsius temp offset
	float K = 273.15;

	int usPerCM = 29;
	int usPerInch = 74;

	// fast square root
	unsigned int root(unsigned int x);
};

#endif //#ifndef ULTRASONIC_H
