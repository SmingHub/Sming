#include <user_config.h>
#include <SmingCore.h>

class LedBlinker
{

public :
	LedBlinker(int reqPin) : ledPin(reqPin) {
		pinMode(ledPin, OUTPUT);
	};
	bool setTimer(int reqInterval) {
		if (reqInterval <= 0) return false;
		ledInterval = reqInterval;
		return true;
	}
	void blink(bool reqRun) {
		if (reqRun) {
			ledTimer.initializeMs(ledInterval, TimerDelegate(&LedBlinker::ledBlink,this)).start();
		}
		else {
			ledTimer.stop();
		}
	}
	void ledBlink () { ledState = !ledState ; digitalWrite(ledPin, ledState);}

private :
	int ledPin = 2;
	Timer ledTimer;
	int ledInterval = 1000;
	bool ledState = true;
};

#define LEDPIN_1  2 // GPIO2
#define LEDPIN_2  4 // GPIO4

LedBlinker myLed1 = LedBlinker(LEDPIN_1);
LedBlinker myLed2 = LedBlinker(LEDPIN_2);

void init()
{
	myLed1.setTimer(1000);
	myLed1.blink(true);
	myLed2.setTimer(500);
	myLed2.blink(true);
}
