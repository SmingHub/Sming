/**
 * Basic Servo test application
 *
*/

#include <SmingCore.h>
#include <Libraries/Servo/ServoChannel.h>

#define noPins 1
#define SERVO_PIN 2 // GPIO2

ServoChannel* channel;

Timer procTimer;
TimerDelegateStdFunction procDelegate;

uint16 centerdelay = 0;
uint32 value = 0;
int degree = 0;

void calcValue()
{
	// wait some time at middle of the servo range
	if(value == 1000) {
		centerdelay--;
		if(centerdelay == 0)
			value += 10;  // restart after waiting
	} else {			  // linear ramp by increasing the value in steps
		centerdelay = 50; // delay 50 times 200ms = 10s
		value += 10;
	}

	if(value >= 2000)
		value = 0; // overflow and restart linear ramp

	Serial.print(" Value: ");
	Serial.print(value);
	Serial.println();

#ifdef raw
	channel->setValue(value); // set all values
#else
	if(degree++ > 90)
		degree = -90;
	channel->setDegree(degree);
#endif
}

void init()
{
	Serial.println("Init Basic Servo Sample");
	System.setCpuFrequency(eCF_80MHz);

	channel = new(ServoChannel);
	channel->attach(SERVO_PIN);

	procDelegate = calcValue;
	procTimer.initializeMs(2000, procDelegate).start();
}
