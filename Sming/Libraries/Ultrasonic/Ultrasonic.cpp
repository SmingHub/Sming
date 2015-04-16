/*
 * Ultrasonic.cpp
 *
 *  Created on: 03 апр. 2015 г.
 *      Author: nikolayba
 */
/*
 Ultrasonic.cpp - Library for HC-SR04 Ultrasonic Ranging Module.library

 Created by ITead studio. Apr 20, 2010.
 iteadstudio.com

 updated by noonv. Feb, 2011
 http://robocraft.ru

 First draft version will be improved
 ported to Sming by sharky Apr, 2015
*/

#include "Ultrasonic.h"


Ultrasonic::Ultrasonic(int TP, int EP)
{
	pinMode(TP, OUTPUT);
	pinMode(EP, INPUT);
	Trig_pin = TP;
	Echo_pin = EP;
}

long Ultrasonic::Timing()
{
	digitalWrite(Trig_pin, LOW);
	delayMicroseconds(2);
	digitalWrite(Trig_pin, HIGH);
	delayMicroseconds(10);
	digitalWrite(Trig_pin, LOW);
	duration = pulseIn(Echo_pin, HIGH);
	return duration;
}

long Ultrasonic::Ranging(int sys)
{
	Timing();
	distacne_cm = duration / 29 / 2;
	distance_inc = duration / 74 / 2;
	if (sys)
		return distacne_cm;
	else
		return distance_inc;
}

