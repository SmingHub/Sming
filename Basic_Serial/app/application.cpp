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
}

void testPrintf()
{
	Serial.printf("Float numbers display test: \n");

	Serial.printf( "Pi with 2 decimals: %.2f and with 4 decimals: %.4f \n", PI, PI);
	Serial.printf( "Pi without specifying precision: %f\n", PI);
	Serial.printf( "A number with leading fraction zeroes like 1.050250=%f\n", 1.050250);

	Serial.printf("Show a string '%s', a decimal %d, a hex %#x, a float with 2 decimals %.2f, " \
			 	 	"unsigned value %u, octal %o.\n",
	        "red", 123456, 0x00C0FFEE, 3.14159, 250, 06675);

	Serial.printf("Test format specifiers on the same parameters\n");

	Serial.printf( "Display -99:%d, -3.010:%f, 'abc'=%s, 3/4:%f, 8/9:%f\n",
			-99, -3.01040, "abc", 3.0/4, 8.0/9);

	Serial.printf( "Display -99:%3d, -3.010:%.3f, 'abc'=%5s, 3/4:%-+.5f, 8/9:%#*f\n",
			-99, -3.01040, "abc", 3.0/4, 8.0/9);

	Serial.printf( "Display -99:%3d, -3.010:%.3f, 'abc'=%#s, 3/4:%5f, 8/9:%.3f\n",
			-99, -3.01040, "abc", 3.0/4, 8.0/9);

	Serial.printf( "Display -99:%d, -3.010:%f, 'abc'=%s, 3/4:%f, 8/9:%f\n",
			-99, -3.01040, "abc", 3.0/4, 8.0/9);

	Serial.printf( "Test invalid specifiers and escaping of '%%': %j %w %%% %%%% % %zk \n");

	Serial.printf("Show int limits %d .. %d \n Show uint limits 0 .. %u \n Show long limits %ld .. %ld \n",
			INT_MIN, INT_MAX, UINT_MAX, LONG_MIN, LONG_MAX);

}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	procTimer.initializeMs(2000, sayHello).start();

	testPrintf();

	/// Reading callback example:
	//  * Option 1
	//	Set Serial Callback to global routine:
	//	   Serial.setCallback(onDataCallback);

	// 	* Option 2
	//  Instantiate hwsDelegateDemo which includes Serial Delegate class
	delegateDemoClass.begin();
}
