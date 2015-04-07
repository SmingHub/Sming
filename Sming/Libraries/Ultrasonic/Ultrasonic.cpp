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
 *    TODO: Fix bugs with sqrt in temp measurement
 */

#include "Ultrasonic.h"

Ultrasonic::Ultrasonic()
{
}

/**
 * Initialize ultrasonic sensor
 */
void Ultrasonic::init(uint8_t trigPin, uint8_t echoPin)
{
	pinTRIG = trigPin;
	pinECHO = echoPin;
	pinMode(pinTRIG, OUTPUT);
	pinMode(pinECHO, INPUT);

	digitalWrite(pinTRIG, LOW);
}

/**
 * Convert ping duration to inches
 *
 * The speed of sound is 1130 feet/s or 73.746 microseconds per inch.
 * The ping travels out and back, so to find the distance to the obstacle we take half of the distance traveled.
 */
long Ultrasonic::us2inch(long microseconds)
{
	return microseconds / usPerInch;
}

/**
 * Convert ping duration to centimeters
 *
 * The speed of sound is 340 m/s or 29 microseconds per centimeter.
 * The ping travels out and back, so to find the distance to the obstacle we take half of the distance traveled.
 */
long Ultrasonic::us2cm(long microseconds)
{
	return microseconds / usPerCM;
}

/**
 * Trigger pulse and wait for echo
 */
long Ultrasonic::ping()
{
	digitalWrite(pinTRIG, LOW);
	delayMicroseconds(2);
	digitalWrite(pinTRIG, HIGH);
	delayMicroseconds(trigDuration);
	digitalWrite(pinTRIG, LOW);
	return pulseIn(pinECHO, HIGH, echoTimeout);
}

/**
 * Measure distance in centimeters
 */
long Ultrasonic::rangeCM()
{
	return us2cm(ping()) / 2;
}

/**
 * Measure distance in inches
 */
long Ultrasonic::rangeInch()
{
	return us2inch(ping()) / 2;
}

/**
 * Calculate temperature in Celsius biasing on known base dist/temp
 *
 * c = sqrt(X*R*T), where:
 * ñ - speed of sound, m/s
 * X - adiabatic index (is about 1.4 for air under normal conditions of pressure and temperature)
 * R - gas constant (for air approximately 8.3145 J/mol·K, ~286.9 J/kg·K)
 * T - the absolute temperature in kelvin
 *
 * @param baseDist	base calibration distance in cm
 * @param baseTemp	base calibration temp (valid for baseDist)
 * @param samples	number of samples for approximation result
 *
 * See http://habrahabr.ru/post/243357/ for details
 */
long Ultrasonic::temp(float baseDist, float baseTemp, int samples)
{
	float dist = 0;
	for (int i = 0; i < samples; i++)
	{
		dist += rangeCM();
		delay(50);
	}

	// avg dist
	dist /= samples;

	// c = sqrt(X*R*T)
	// TODO !!! Draft for compile, in root need to check types/overflows/signs here
	float speedOfSound = baseDist / dist * root(XR * (K + baseTemp));

	// T = (c*c)/(X*R) in Kelvin
	return (speedOfSound * speedOfSound) / (XR) - K;
}

unsigned int Ultrasonic::root(unsigned int x)
{
	unsigned int a, b;
	b = x;
	a = x = 0x3f;
	x = b / x;
	a = x = (x + a) >> 1;
	x = b / x;
	a = x = (x + a) >> 1;
	x = b / x;
	x = (x + a) >> 1;
	return (x);
}
