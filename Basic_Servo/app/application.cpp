/**
 * Basic Servo test application
 *
*/

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/HardwareServo/HardwareServoChannel.h>

#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif


#define noPins 1
#define SERVO_PIN 2 // GPIO2

HardwareServoChannel *channel;

Timer procTimer;
TimerDelegate procDelegate;

uint16 centerdelay = 0;
uint32 value = 0;
int degree=0;


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

#ifdef raw
	channel->setValue(value);				// set all values
#else
	if(degree++>90) degree=-90;
	channel->setDegree(degree);
#endif
}


#define testCallTrue(func,comment) Serial.print(# func comment "\t"); Serial.println(channel->func()?" OK":" NOK");
#define testFuncTrue(func,comment, val) Serial.print(# func comment "\t"); Serial.println(channel->func(val)?" OK":" NOK");
#define testGetTrue(func,comment, val) Serial.print(# func comment "\t"); Serial.println(channel->func()==val?" OK":" NOK");
#define testSetTrue(func,comment, val) Serial.print(# func comment "\t"); Serial.println(channel->func(val)?" OK":" NOK");
#define testGetFalse(func,comment, val) Serial.print(# func comment "\t"); Serial.println(channel->func()==val?" NOK":" OK");
#define testSetFalse(func,comment, val) Serial.print(# func comment "\t"); Serial.println(channel->func(val)?" NOK":" OK");

void testStateMachine () {
	static uint16 state =0;

	switch (state) {
	case 0:			// Wait until connected
		if (WifiStation.isConnected()) state = 10;
		break;
	case 10:
		channel = new(HardwareServoChannel);


		// read all getter after construction
		testGetTrue( getMaxValue," 2300", 2300);
		testGetTrue( getMinValue,"  700", 700);
		testGetTrue( getPin,"         0", 0);
		testGetTrue( getValue,"       0", 0);

		// attach and detach
		testFuncTrue(attach,"          ", SERVO_PIN);
		testCallTrue(detach,"          ");

		state = 20;	// wait to see that interrupt is really stopped (otherwise should raise assertion)
		break;
	case 20:
		// reattach
		testFuncTrue(attach,"     again", SERVO_PIN);

		// test setMax function
		testCallTrue(setMaxValue," init");
		testGetTrue( getMaxValue," 2300", 2300);

		testSetTrue( setMaxValue," 2200", 2200);
		testGetTrue( getMaxValue," 2200", 2200);

		testSetFalse(setMaxValue,"< min", 699);
		testGetTrue( getMaxValue," 2200", 2200);

		// test setMin function
		testCallTrue(setMinValue," init");
		testGetTrue( getMinValue,"  700", 700);

		testSetTrue( setMinValue,"  500", 500);
		testGetTrue( getMinValue,"  500", 500);

		testSetFalse(setMinValue," >max", 2300);
		testGetTrue( getMinValue,"  500", 500);

		// test setValue function
		testSetFalse(setValue,"    >max", 1701);
		testGetTrue( getValue,"       0", 0);
		testSetTrue( setValue,"    1700", 1700);
		testGetTrue( getValue,"    1700", 1700);

		// reduce max and affect value because of boundary
		testSetTrue( setMaxValue," 2000", 2000);
		testGetTrue( getMaxValue," 2000", 2000);
		testGetTrue( getValue,"    1500", 1500);

		// increase min and affect value because of boundary
		testSetTrue( setMinValue,"  700", 700);
		testGetTrue( getMinValue,"  700", 700);
		testGetTrue( getValue,"    1300", 1300);

		// test setDegree function
		testSetTrue( setDegree,"     90", 90);
		testGetTrue( getValue,"    1300", 1300);

		testSetTrue( setDegree,"      0", 0);
		testGetTrue( getValue,"     650", 650);

		testSetTrue( setDegree,"    -90", -90);
		testGetTrue( getValue,"       0", 0);

		testSetFalse(setDegree,"    -91", -91);
		testSetFalse(setDegree,"     91", 91);

		state = 100;
		break;
	case 100:	// test movement
		calcValue();
		break;
	default:
		break;
	}
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
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

	procDelegate = testStateMachine;
	procTimer.initializeMs(2000,procDelegate).start();

}

