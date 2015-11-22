/**
 * Basic Servo test application
 *
*/

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "HardwareServo.h"

#define noPins 4
#define SERVO_PIN 2 // GPIO2


Timer procTimer;
uint8 pins[noPins] = {SERVO_PIN,SERVO_PIN,SERVO_PIN,SERVO_PIN};

uint32 value[noPins] = {0,0,0,0};
void calcValue() {
	value[0] +=10;

	if (value[0] >= 1000) value[0] = 0;

	for (uint8 i=0; i<noPins; i++) {
		value[i] = value[0]+i;
		Serial.print(" Value: ");
		Serial.print(value[i]);
	}
	Serial.println();
	hardwareServo.SetValues(value);
}

TimerDelegate calcDelegate = calcValue;


// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
	hardwareServo.Init(pins,noPins);
	procTimer.initializeMs(2000,calcDelegate).start();
}

void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail);
}

void init()
{
	WifiStation.enable(true);
	System.setCpuFrequency(eCF_80MHz);

	Serial.print("Init");
	Serial.println();
	WifiStation.waitConnection(connectOk, 30, connectFail);
}

