/*
 * Servo.cpp
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */

#include "Servo.h"

// DEBUG for visual check of impulses with a LED instead of the servo
//#define DEBUG
#ifdef DEBUG
#define FACTOR 500
#else
#define FACTOR 1
#endif

#define SERVO_PERIOD 20000

HardwareTimer Servo::hardwareTimer;
uint8 Servo::pins[SERVO_CHANNEL_NUM_MAX];
uint32 Servo::timing[SERVO_CHANNEL_NUM_MAX * 2 + 1];
uint8 Servo::maxTimingIdx = 0;
uint8 Servo::actIndex = 0;
bool Servo::started = false;

void IRAM_ATTR Servo::timerInt()
{
	hardwareTimer.setInterval(timing[actIndex]);
	hardwareTimer.startOnce();

	if(actIndex < maxTimingIdx) {
		bool out = !(actIndex % 2);
		digitalWrite(pins[actIndex / 2], out);
	}

	++actIndex;
	if(actIndex > maxTimingIdx) {
		actIndex = 0;
	}
}

Servo::Servo() : channels(SERVO_CHANNEL_NUM_MAX, 0)
{
	for(unsigned i = 0; i < ARRAY_SIZE(timing); i++) {
		timing[i] = hardwareTimer.usToTicks<1000>();
	}
}

bool Servo::addChannel(ServoChannel* channel)
{
	uint8 channel_count = channels.size();
	if(channel_count >= SERVO_CHANNEL_NUM_MAX) {
		return false;
	}
	channels.add(channel);

	ETS_INTR_LOCK();
	getPins();
	calcTiming();
	ETS_INTR_UNLOCK();

	if(!started) {
		started = true;
		hardwareTimer.initializeUs<100000>(timerInt);
		hardwareTimer.startOnce();
	}
	return true;
}

bool Servo::removeChannel(ServoChannel* channel)
{
	if(channels.removeElement(channel)) {
		ETS_INTR_LOCK();
		getPins();
		calcTiming();
		ETS_INTR_UNLOCK();
		if(channels.size() == 0) {
			hardwareTimer.stop();
			started = false;
		}
		return true;
	}
	return false;
}

void Servo::calcTiming()
{
	auto usToTicks = [this](uint32_t us) { return hardwareTimer.usToTicks(us * FACTOR); };

	uint32 sumTime = 0;
	unsigned channel_count = channels.size();
	for(unsigned i = 0; i < channel_count; i++) {
		uint32 onTime = channels[i]->getValue() + channels[i]->getMinValue();
		timing[i * 2 + 0] = usToTicks(onTime);
		sumTime += onTime;

		uint32 offTime = channels[i]->getMaxValue() - onTime;
		timing[i * 2 + 1] = usToTicks(offTime);
		sumTime += offTime;
	}

	uint32 frameTime = SERVO_PERIOD - sumTime;
	timing[channel_count * 2] = usToTicks(frameTime);

	maxTimingIdx = channel_count * 2;
}

void Servo::getPins()
{
	for(unsigned i = 0; i < channels.size(); i++) {
		ServoChannel* ch = channels.get(i);
		if(ch != nullptr) {
			pins[i] = ch->getPin();
		}
	}
}

Servo servo;
