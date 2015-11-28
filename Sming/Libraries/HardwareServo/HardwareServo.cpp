/*
 * HardwareServo.cpp
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */

#include "HardwareServo.h"
#include <HardwareTimer.h>

// DEBUG for visual check of impulses with a LED instead of the servo
//#define DEBUG
#ifdef DEBUG
#define FACTOR 500
#endif

#define SERVO_PERIOD 20000


Hardware_Timer hardwareTimer;

uint8 pins[SERVO_CHANNEL_NUM_MAX];
uint32 timing[SERVO_CHANNEL_NUM_MAX*2+1];
uint8 maxTimingIdx;
uint8 actIndex;
bool started = false;

void IRAM_ATTR ServoTimerInt()
{
	assert(started);
	hardwareTimer.setIntervalUs(timing[actIndex]);
	hardwareTimer.startOnce();

	if (actIndex < maxTimingIdx) {
		bool out = !(actIndex%2);
		digitalWrite(pins[actIndex/2],out);
	}

	if (++actIndex > maxTimingIdx) actIndex = 0;
}

HardwareServo::HardwareServo()
{
	for(uint8 i=0; i<SERVO_CHANNEL_NUM_MAX*2+1; i++) timing[i] = 1000;
	maxTimingIdx=0;
}

HardwareServo::~HardwareServo()
{
	channels.removeAllElements();
}

bool HardwareServo::addChannel(HardwareServoChannel* channel)
{
	uint8 channel_count = channels.size();
	if (channel_count > SERVO_CHANNEL_NUM_MAX) return false;
	channels.add(channel);

	ETS_INTR_LOCK();
	getPins();
	calcTiming();
	ETS_INTR_UNLOCK();

	if (!started) {
		started = true;
		hardwareTimer.initializeUs(100000,ServoTimerInt);
		hardwareTimer.startOnce();
	}
	return true;
}

bool HardwareServo::removeChannel(HardwareServoChannel* channel)
{
	if (channels.removeElement(channel)) {
		ETS_INTR_LOCK();
		getPins();
		calcTiming();
		ETS_INTR_UNLOCK();
		if (channels.size() == 0) {
			started = false;
			hardwareTimer.stop();
		}
		return true;
	}
	return false;
}

void HardwareServo::calcTiming()
{
	uint32 sumTime=0;
	uint8 channel_count = channels.size();
	for (uint8 i = 0; i < channel_count; i++) {
		uint32 onTime = channels[i]->getValue() + channels[i]->getMinValue();
		timing[i*2+0] = onTime;
		sumTime += onTime;

		uint32 offTime = channels[i]->getMaxValue() - onTime;
		timing[i*2+1] = offTime;
		sumTime += offTime;

#ifdef DEBUG
		timing[i*2+0] *= FACTOR;
		timing[i*2+1] *= FACTOR;
#endif
	}

	uint32 frameTime= SERVO_PERIOD - sumTime;
	timing[channel_count*2] = frameTime;

#ifdef DEBUG
	timing[channel_count*2] *= FACTOR;
#endif

	maxTimingIdx = channel_count*2;
}


void HardwareServo::getPins()
{
	for (uint8 i=0;i<channels.size();i++) {
		HardwareServoChannel *ch = channels.get(i);
		if (ch != null) {
			pins[i++] = ch->getPin();
		}
	}
}

HardwareServo hardwareServo;

