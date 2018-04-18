
//   Created on: 7 Jan 2016
//       Author: Frank
//

#include <SmingCore/SmingCore.h>
#include "LedBlinker.h"

// ======================
// LED Blinker
// ======================

void 	onTimer_blink();
Timer 	procTimer_blink;

struct blinkInterval
{
	short	nbrOfPeriods;
	short	period[3 * 2];
};

blinkInterval blinkSlow		= { .nbrOfPeriods = 1 * 2,{ 920, 80 } };						// ........b.........b
blinkInterval blinkTwice	= { .nbrOfPeriods = 2 * 2,{ 1000, 80, 170, 80 } };				// ........b.b........b.b
blinkInterval blink3times	= { .nbrOfPeriods = 3 * 2,{ 1000, 170, 200, 170, 200, 170 } };	// ........b.b.b.......b.b.b
blinkInterval blinkFast		= { .nbrOfPeriods = 1 * 2,{ 150, 80 } };						// ..b..b..b..b..b..b..b..b..

blinkInterval blinkCurrent = blinkSlow;

int blinkCtr = 0;
int blinkState = 0;


// Entry point for initialisation 
void LedBlinker::setup(int heartbeatLED)
{
	mheartbeatLED = heartbeatLED;
	pinMode(heartbeatLED, OUTPUT);
	// Start the heartbeat LED blinking
	procTimer_blink.initializeMs(500, TimerDelegate(&LedBlinker::onTimer_blink, this)).start();
}

void LedBlinker::speed(LedBlinker::Speed speed)
{
	switch (speed)
	{
	case  Fast:			blinkCurrent = blinkSlow;	break;
	case  Twice: 		blinkCurrent = blinkTwice;	break;
	case  Threetimes: 	blinkCurrent = blink3times;	break;
	case  Slow:
	default:			blinkCurrent = blinkFast;	break;
	}
}

void LedBlinker::onTimer_blink()
{
	blinkCtr++;
	int i = blinkCtr % blinkCurrent.nbrOfPeriods;
	int state = ((i % 2) == 0) ? LOW : HIGH;
	int duration = blinkCurrent.period[i];

	digitalWrite(mheartbeatLED, state);
	procTimer_blink.setIntervalMs(duration);
}

