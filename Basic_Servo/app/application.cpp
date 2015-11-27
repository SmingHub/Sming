/**
 * Basic Servo test application
 *
*/

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "HardwareServoChannel.h"

#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif


#define noPins 1
#define SERVO_PIN 2 // GPIO2

HardwareServoChannel *channel;
Timer procTimer;
uint16 centerdelay = 0;
uint32 value = 0;


void calcValue() {
	// wait some time at middle of the servo range
	if (value == 800) {
		centerdelay--;
		if (centerdelay == 0) value +=10;	// restart after waiting
	}
	else {										// linear ramp by increasing the value in steps
		centerdelay=50;							// delay 50 times 200ms = 10s
		value +=10;
	}


	if (value >= 1600) value = 0;			// overflow and restart linear ramp

	Serial.print(" Value: ");
	Serial.print(value);
	Serial.println();

	channel->setValue(value);				// set all values
}

TimerDelegate calcDelegate = calcValue;


// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
	channel = new(HardwareServoChannel);
	channel->attach(SERVO_PIN);
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
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	System.setCpuFrequency(eCF_80MHz);

	Serial.print("Init Basic Servo Sample");
	Serial.println();
	WifiStation.waitConnection(connectOk, 30, connectFail);
}

