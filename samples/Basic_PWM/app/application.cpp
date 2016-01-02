#include <user_config.h>
#include <SmingCore/SmingCore.h>

#define LED_PIN 12 // GPIO12

DriverPWM ledPWM;
Timer procTimer;

int i = 0;
bool countUp = true;
bool countDown = false;

void doPWM()
{
	if(countUp){
		i++;
		if(i == 100){
			countUp = false;
			countDown = true;
		}
	}
	else{
		i--;
		if(i == 0){
			countUp = true;
			countDown = false;
		}
	}
	Serial.println(i);
	ledPWM.analogWrite(LED_PIN, i);
}

void init()
{
	ledPWM.initialize();
	procTimer.initializeMs(10, doPWM).start();
}

