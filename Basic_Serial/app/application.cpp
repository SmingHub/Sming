#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "SerialReadingDelegateDemo.h"

Timer procTimer;
SerialReadingDelegateDemo delegateDemoClass;

void sayHello()
{
	Serial.print("Hello Sming! Let's do smart things.");
	Serial.print(" Time : ");
	Serial.println(micros());
	Serial.println();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	procTimer.initializeMs(2000, sayHello).start();

	/// Reading callback example:
	//  * Option 1
	//	Set Serial Callback to global routine:
	//	   Serial.setCallback(onDataCallback);

	// 	* Option 2
	//  Instantiate hwsDelegateDemo which includes Serial Delegate class
	delegateDemoClass.begin();
}
