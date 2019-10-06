/*
 * ServoChannel.cpp
 *
 *  Created on: 27.11.2015
 *      Author: johndoe
 */

#include "ServoChannel.h"
#include "Servo.h"
#include <muldiv.h>

bool ServoChannel::setMaxValue(uint32_t maxValue)
{
	if(maxValue < minValue || maxValue > Servo::maxChannelTime) {
		return false;
	}

	this->maxValue = maxValue;

	if(value > maxValue) {
		value = maxValue;
		servo.updateChannel(this);
	}

	return true;
}

bool ServoChannel::setMinValue(uint32_t minValue)
{
	if(minValue > maxValue || minValue < Servo::minChannelTime) {
		return false;
	}

	this->minValue = minValue;

	if(value < minValue) {
		value = minValue;
		servo.updateChannel(this);
	}

	return true;
}

uint32_t ServoChannel::setValue(uint32_t value)
{
	if(value < minValue) {
		value = minValue;
	} else if(value > maxValue) {
		value = maxValue;
	}
	if(value != this->value) {
		this->value = value;
		servo.updateChannel(this);
	}
	return value;
}

bool ServoChannel::attach(uint8_t pin)
{
	this->pin = pin;
	return servo.addChannel(this);
}

bool ServoChannel::detach()
{
	return servo.removeChannel(this);
}

static uint32_t map32(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max)
{
	return muldiv(x - in_min, out_max - out_min, in_max - in_min) + out_min;
}

uint32_t ServoChannel::setDegree(int8_t value)
{
	if(value < -90) {
		value = -90;
	} else if(value > 90) {
		value = 90;
	}
	return setValue(map32(value, -90, 90, minValue, maxValue));
}
