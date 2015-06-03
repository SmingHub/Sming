#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "SerialReadingDelegateDemo.h"

Timer procTimer;
SerialReadingDelegateDemo delegateDemo;

void sayHello()
{
	Serial.print("Hello Sming! Let's do smart things.");
	Serial.print(" Time : ");
	Serial.println(micros());
	Serial.println();
}

void onDataCallback(Stream& stream, char recvChar, unsigned short charCount)
{

}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	procTimer.initializeMs(2000, sayHello).start();

	/// Reading callback example:
	//  * Option 1
	//	Set Serial Callback to global routine:
	//     void onDataCallback(Stream& stream, unsigned short charCount)
	//	   Serial.setCallback(onDataCallback);

	// 	* Option 2
	//  Instantiate hwsDelegateDemo which includes Serial Delegate class
	delegateDemo.begin();
}
