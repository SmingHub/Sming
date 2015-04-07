/*
 * Sming library: Ultrasonic sonar modules driver
 *
 * Tested on modules:
 *   HC-SR04 - ranges: 2-400cm, power: 5v, levels: TTL, for work with 3.3v need voltage divider on ECHO pin
 *
 *    Author: sharky <nik.sharky@gmail.com>
 * Thanks to: ITead studio, DIMOSUS, ElecFreaks, Wiki, Parallax for information and ideas
 *
 * Changed: 2015-04-07
 *    TODO: Add multisampling to ping and range methods
 *    TODO: Add setMedium method to set current medium parameters
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
	void init(uint8_t trigPin, uint8_t echoPin);
	long ping();
	long us2cm(long microseconds);
	long us2inch(long microseconds);
	long rangeCM();
	long rangeInch();
	long temp(float baseDist, float baseTemp, int samples = 50);

private:
	int pinTRIG;
	int pinECHO;

	// Duration of trig pulse (microseconds)
	unsigned long trigDuration = 10;

	// Timeout for receiving echo after trig pulse (microseconds)
	// 100ms by default (spec recommend ~60ms, need to check stability)
	unsigned long echoTimeout = 100000L;

	// Õ - adiabatic index of medium (is about 1.4 for air under normal conditions of pressure and temperature)
	// R - gas constant for medium (for air approximately 8.3145 J/mol·K, ~286.9 J/kg·K)
	// See http://www.engineeringtoolbox.com/individual-universal-gas-constant-d_588.html for details
	float X = 1.4;
	float R = 286.9;
	float XR = X * R;

	// Kelvin/Celsius temp offset
	float K = 273.15;

	int usPerCM = 29;
	int usPerInch = 74;
};

#endif //#ifndef ULTRASONIC_H
