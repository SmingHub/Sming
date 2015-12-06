/*
 * ServoChannel.cpp
 *
 *  Created on: 27.11.2015
 *      Author: johndoe
 */

#include "ServoChannel.h"

#include "Servo.h"

ServoChannel::ServoChannel()
{
	pin = 0;
	value = 0;
	maxValue=DEFAULTMAXVALUE;
	minValue=DEFAULTMINVALUE;
}

ServoChannel::~ServoChannel()
{
	// TODO Auto-generated destructor stub
}

void ServoChannel::plausValue()
{
	uint32 range = maxValue-minValue;
	if (value > range) {
		value = range;
		servo.calcTiming();
	}
}

bool ServoChannel::setMaxValue(uint32 maxValue)
{
	if (maxValue < minValue) return false;
	this->maxValue = maxValue;
	plausValue();
	return true;
}

bool ServoChannel::setMinValue(uint32 minValue )
{
	if (minValue > maxValue) return false;
	this->minValue = minValue;
	plausValue();
	return true;
}

uint32 ServoChannel::getValue() const
{
	return value;
}

bool ServoChannel::setValue(uint32 value)
{
	if (value > maxValue-minValue) return false;
	this->value = value;
	servo.calcTiming();
	return true;
}

bool ServoChannel::attach(uint8 pin)
{
	this->pin = pin;
	if (servo.addChannel(this)) {
		pinMode(pin, OUTPUT);
		return true;
	} else {
		return false;
	}
}

bool ServoChannel::detach()
{
	if (servo.removeChannel(this)) {
		pinMode(pin, INPUT);
		return true;
	} else {
		return false;
	}
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


bool ServoChannel::setDegree(int8 value)
{
	if ((value < -90) || (value > 90)) return false;
	setValue(map(value,-90,90,0,maxValue-minValue));
	return true;
}
