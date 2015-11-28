/*
 * HardwareServoChannel.cpp
 *
 *  Created on: 27.11.2015
 *      Author: johndoe
 */

#include "HardwareServoChannel.h"
#include <HardwareServo.h>

HardwareServoChannel::HardwareServoChannel()
{
	pin = 0;
	value = 0;
	maxValue=DEFAULTMAXVALUE;
	minValue=DEFAULTMINVALUE;
}

HardwareServoChannel::~HardwareServoChannel()
{
	// TODO Auto-generated destructor stub
}

void HardwareServoChannel::plausValue()
{
	uint32 range = maxValue-minValue;
	if (value > range) {
		value = range;
		hardwareServo.calcTiming();
	}
}

bool HardwareServoChannel::setMaxValue(uint32 maxValue)
{
	if (maxValue < minValue) return false;
	this->maxValue = maxValue;
	plausValue();
	return true;
}

bool HardwareServoChannel::setMinValue(uint32 minValue )
{
	if (minValue > maxValue) return false;
	this->minValue = minValue;
	plausValue();
	return true;
}

uint32 HardwareServoChannel::getValue() const
{
	return value;
}

bool HardwareServoChannel::setValue(uint32 value)
{
	if (value > maxValue-minValue) return false;
	this->value = value;
	hardwareServo.calcTiming();
	return true;
}

bool HardwareServoChannel::attach(uint8 pin)
{
	this->pin = pin;
	if (hardwareServo.addChannel(this)) {
		pinMode(pin, OUTPUT);
		return true;
	} else {
		return false;
	}
}

bool HardwareServoChannel::detach()
{
	if (hardwareServo.removeChannel(this)) {
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


bool HardwareServoChannel::setDegree(int8 value)
{
	if ((value < -90) || (value > 90)) return false;
	setValue(map(value,-90,90,0,maxValue-minValue));
	return true;
}
