#include <user_config.h>
#include <SmingCore.h>


void plainOldOrdinaryFunction()
{
	debugf("plainOldOrdinaryFunction");
}

class LedBlinker
{

public :
	LedBlinker(int reqPin) : ledPin(reqPin) {
		pinMode(ledPin, OUTPUT);
	};
	bool setTimer(int reqInterval) {
		if (reqInterval <= 0) 
			return false;
		ledInterval = reqInterval;
		return true;
	}

	// This example show the way delegates have been used in Sming in the past.
	void blinkOldDelegate(bool reqRun) {
		if (reqRun) {
			ledTimer.initializeMs(ledInterval, TimerDelegate(&LedBlinker::ledBlink, this)).start();
		}
		else {
			ledTimer.stop();
		}
	}

	// This example shows how to use a plain old ordinary function as a callback
	void callPlainOldOrdinaryFunction(bool reqRun) {
		if (reqRun) {
			ledTimer.initializeMs(ledInterval, TimerDelegateStdFunction(plainOldOrdinaryFunction)).start();
			// or just
			// ledTimer.initializeMs(ledInterval, plainOldOrdinaryFunction).start();
		}
		else {
			ledTimer.stop();
		}
	}


	// Sming now allows the use of std::function
	// This example shows how to use a lamda expression as a callback
	void callLamda(bool reqRun) {
		if (reqRun) {
			int foo = 123;
			ledTimer.initializeMs(ledInterval, 
				[&]	// capture everything by reference - so foo will be available inside the lamda
				()  // No parameters to the callback
				{
					debugf("lamda Callback ");
					if (foo > 123)
						debugf("foo is 123");
			
				})
				.start();
		}
		else {
			ledTimer.stop();
		}
	}



	// This example shows how to use a member function as a callback
	void callMemberFunction(bool reqRun) {
		if (reqRun) {

			// A non-static member function must be called with an object. 
			// That is, it always implicitly passes "this" pointer as its argument.
			// But because our callback specifies that we don't take any arguments (<void(void)>), 
			// you must use std::bind to bind the first (and the only) argument.

			TimerDelegateStdFunction b = std::bind(&LedBlinker::callbackMemberFunction, this);
			ledTimer.initializeMs(ledInterval, b).start();
		}
		else {
			ledTimer.stop();
		}
	}

	void ledBlink()
	{
		ledState = !ledState;
		digitalWrite(ledPin, ledState);
	}
	void callbackMemberFunction()
	{
		debugf("callMemberFunction");
	}
	

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

LedBlinker myLed3 = LedBlinker(0);
LedBlinker myLed4 = LedBlinker(0);
LedBlinker myLed5 = LedBlinker(0);

void init()
{
	myLed1.setTimer(1000);
	myLed1.blinkOldDelegate(true);
	
	myLed2.setTimer(500);
	myLed2.blinkOldDelegate(true);


	myLed3.setTimer(1000);
	myLed3.callPlainOldOrdinaryFunction(true);

	myLed4.setTimer(1000);
	myLed4.callMemberFunction(true);

	myLed5.setTimer(1000);
	myLed5.callLamda(true);

}
