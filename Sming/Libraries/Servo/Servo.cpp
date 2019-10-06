/*
 * Servo.cpp
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */

#include "Servo.h"
#include <Digital.h>

Servo servo;

void Servo::staticTimerIsr()
{
	servo.timerIsr();
}

void Servo::timerIsr()
{
	auto& frame = frames[activeFrameIndex];
	hardwareTimer.setInterval(frame.slots[activeSlot]);
	if(activeSlot > 0) {
		digitalWrite(frame.pins[activeSlot - 1], false);
	}
	++activeSlot;
	if(activeSlot < frame.slotCount) {
		digitalWrite(frame.pins[activeSlot - 1], true);
	} else {
		activeFrameIndex = nextFrameIndex;
		activeSlot = 0;
	}
}

Servo::Servo()
{
	HardwareTimer::checkIntervalUs<framePeriod>();
	updateTimer.initializeUs<framePeriod / 2>([]() { servo.update(); });
}

int Servo::findChannel(ServoChannel* channel)
{
	for(unsigned i = 0; i < maxChannels; ++i) {
		if(channels[i] == channel) {
			return i;
		}
	}
	return -1;
}

bool Servo::addChannel(ServoChannel* channel)
{
	if(channel == nullptr) {
		return false;
	}

	int i = findChannel(channel);
	if(i >= 0) {
		return true; // Already added
	}

	// Find a free channel
	i = findChannel(nullptr);
	if(i < 0) {
		// All channels in use
		return false;
	}

	channels[i] = channel;
	++channelCount;

	pinMode(channel->getPin(), OUTPUT);

	updateTimer.startOnce();
	return true;
}

bool Servo::removeChannel(ServoChannel* channel)
{
	if(channel == nullptr) {
		return false;
	}

	int i = findChannel(channel);
	if(i < 0) {
		// Channel not active
		return false;
	}

	pinMode(channel->getPin(), INPUT);

	channels[i] = nullptr;
	--channelCount;

	updateTimer.startOnce();
	return true;
}

void Servo::updateChannel(ServoChannel* channel)
{
	updateTimer.startOnce();
}

void Servo::update()
{
	if(channelCount == 0) {
		hardwareTimer.stop();
	} else {
		calcTiming();
		if(!hardwareTimer.isStarted()) {
			// Start after brief arbitrary delay
			activeSlot = 0;
			hardwareTimer.initializeUs<500>(staticTimerIsr);
			hardwareTimer.start();
		}
	}
}

void Servo::calcTiming()
{
	unsigned newFrameIndex;
	if(hardwareTimer.isStarted()) {
		newFrameIndex = (nextFrameIndex + 1) % ARRAY_SIZE(frames);
		if(newFrameIndex == activeFrameIndex) {
			// Change pending, try again later
			updateTimer.startOnce();
			return;
		}
	} else {
		newFrameIndex = activeFrameIndex;
	}

	auto& frame = frames[newFrameIndex];
	unsigned slotCount = 0;

	uint32_t totalTicks = 0;
	for(unsigned i = 0; i < maxChannels; ++i) {
		auto channel = channels[i];
		if(channel == nullptr) {
			continue;
		}

		frame.pins[slotCount] = channel->getPin();
		auto ticks = HardwareTimer::usToTicks(channel->getValue());
		frame.slots[slotCount] = ticks;
		totalTicks += ticks;
		++slotCount;
	}
	frame.slots[slotCount++] = periodTicks - totalTicks;
	frame.slotCount = slotCount;

	// ISR will switch at end of next frame
	nextFrameIndex = newFrameIndex;
}
