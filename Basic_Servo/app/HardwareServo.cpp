/*
 * HardwareServo.cpp
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */

#include "HardwareServo.h"
#include <HWTimer.h>

// DEBUG for visual check of impulses with a LED instead of the servo
//#define DEBUG
#ifdef DEBUG
#define FACTOR 500
#else
#define FACTOR 1
#endif

#define SERVO_PERIOD (20000*FACTOR)
#define SERVO_MIN (700*FACTOR)
#define SERVO_MAX (2300*FACTOR)

HardwareServo::HardwareServo()
{
	memset(values,0,sizeof(values));
	channel_count=0;
}

HardwareServo::~HardwareServo()
{
}

uint8 pin[SERVO_CHANNEL_NUM_MAX];
uint32 timing[SERVO_CHANNEL_NUM_MAX*2+1];
uint8 maxTimingIdx;
uint8 actIndex;

void IRAM_ATTR ServoTimerInt()
{
	hwTimer.setIntervalUs(timing[actIndex]);
	hwTimer.startOnce();

	if (actIndex < maxTimingIdx) {
		bool out = !(actIndex%2);
		digitalWrite(pin[actIndex/2],out);
	}

	if (++actIndex > maxTimingIdx) actIndex = 0;

#ifdef DEBUG
	Serial.print("Interrupt");
#endif
}

void HardwareServo::calcTiming()
{
	for (uint8 i = 0; i < channel_count; i++) {
		uint32 onTime = values[i] + SERVO_MIN;
		timing[i*2+0] = onTime;

		uint32 offTime = SERVO_MAX - onTime;
		timing[i*2+1] = offTime;

	}
	maxTimingIdx = channel_count*2;
	timing[maxTimingIdx] = SERVO_PERIOD - SERVO_MAX*channel_count;
}

void HardwareServo::Init(uint8 *pins, uint8 no_of_pins)
{
	if (no_of_pins > SERVO_CHANNEL_NUM_MAX) {
		channel_count = 0;
		return;
	}
	channel_count = no_of_pins;
	if (no_of_pins > 0) {
		for (uint8 i = 0; i < no_of_pins; i++) {
			pinMode(pins[i], OUTPUT);
			pin[i] = pins[i];
		}
		calcTiming();
		isrServo = ServoTimerInt;
		hwTimer.initializeUs(100000,isrServo);
		hwTimer.startOnce();
	}
}


bool HardwareServo::SetValues(uint32 value[])
{

	for (uint8 i=0; i < channel_count; i++) {
		if (value[i] >= SERVO_MAX-SERVO_MIN) return false;
		values[i] = value[i];
	}
	calcTiming();
	return true;
}


HardwareServo hardwareServo;
