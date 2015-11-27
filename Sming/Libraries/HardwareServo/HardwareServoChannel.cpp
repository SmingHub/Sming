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
	maxValue=2300;
	minValue=700;
}

HardwareServoChannel::~HardwareServoChannel()
{
	// TODO Auto-generated destructor stub
}

bool HardwareServoChannel::setMaxValue(uint32 maxValue)
{
	if (maxValue < minValue) return false;
	this->maxValue = maxValue;
	return true;
}

bool HardwareServoChannel::setMinValue(uint32 minValue )
{
	if (minValue > maxValue) return false;
	this->minValue = minValue;
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

void HardwareServoChannel::attach(uint8 pin)
{
	pinMode(pin, OUTPUT);
	this->pin = pin;
	hardwareServo.addChannel(this);
}

void HardwareServoChannel::detach()
{
	hardwareServo.removeChannel(this);
}

bool HardwareServoChannel::setDegree(int8 value)
{
	if ((value < -90) || (value > 90)) return false;
	setValue(map(value,-90,90,minValue,maxValue));
	return true;
}
