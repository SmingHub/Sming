#include <SmingCore.h>
#include <user_config.h>

class LedBlinker {
 public:

  LedBlinker(int reqPin) : ledPin(reqPin) { pinMode(ledPin, OUTPUT); };
  
  bool setTimer(int reqInterval) {

    return reqInterval <= 0 ? false : (ledInterval = reqInterval);
  }
  
  void blink(bool reqRun) {

    if (!reqRun)  return ledTimer.stop();
  
    ledTimer
          .initializeMs(ledInterval, TimerDelegate(&LedBlinker::ledBlink, this))
          .start();
  }

  void ledBlink() { digitalWrite(ledPin, ledState = !ledState); }

	private:
		bool 	ledState = true;
		int 	  ledPin = 2,
			 ledInterval = 1000;
		Timer ledTimer;
};

#define LEDPIN_1 2  // GPIO2
#define LEDPIN_2 4  // GPIO4

LedBlinker myLed1 = LedBlinker(LEDPIN_1),
					 myLed2 = LedBlinker(LEDPIN_2);

void init() {
  myLed1.setTimer(1000);
  myLed1.blink(true);
  myLed2.setTimer(500);
  myLed2.blink(true);
}
