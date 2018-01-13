/*
 * application.h
 *
 *      Author: frank
 */

#pragma once

// ===================================================================
// Binking a LED is a common way to indicate we are alive and kicking.
// But the way the LED blinks can convey other information - error states and so on.
// This class supports 4 different ways to blink
// ===================================================================


class LedBlinker
{
public:
	LedBlinker(){}
	enum Speed
	{
		Slow		= 0
		,Twice		= 1
		,Threetimes = 2
		,Fast		= 3
	};
	void setup(int heartbeatLED);
	void speed(Speed speed);
private:
	void	onTimer_blink();
	int		mheartbeatLED;
};
