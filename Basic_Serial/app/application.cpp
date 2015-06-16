#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "SerialReadingDelegateDemo.h"

Timer procTimer;
SerialReadingDelegateDemo delegateDemoClass;
int helloCounter = 0;

void sayHello()
{
	Serial.print("Hello Sming! Let's do smart things.");
	Serial.print(" Time : ");
	Serial.println(micros());
	Serial.println();

	Serial.printf("This is Hello message %d \r\n", ++helloCounter);
	/*
	if ((helloCounter % 3) == 0)
	{
		if (Debug.status())
		{
			Debug.stop();
		}
		else
		{
			Debug.start();
		}
	}
	debugf("Debug from sayHello %d", helloCounter);
}
*/
	Debug.lprintf((helloCounter) % 4, "This is a info/warning/debug message %d, debug level = %d \r\n", helloCounter, Debug.getDebugLevel());;
	lprintf2((helloCounter % 2), "lprintf2 %d\r\n", helloCounter);

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
