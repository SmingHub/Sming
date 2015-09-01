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
	Serial.printf("\n== PRINTF TEST START ==\n");

	Serial.printf("\nFloat numbers display test: \n");

	Serial.printf( "Pi with 2 decimals: %.2f and with 4 decimals: %.4f \n", PI, PI);
	Serial.printf( "Pi without specifying precision(default 9): %f\n", PI);
	Serial.printf( "A number with leading fraction zeroes like 1.050250=%f\n", 1.050250);

	Serial.printf("\nTest crazy line lengths: wait for it, wait for it, wait for it, wait for it, "\
			"wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, " \
			"wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, " \
			"wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, got to the end :)\n");

	Serial.printf("\nShow a decimal %d, a hex %#x, an unsigned %u, an octal %o.\n",
	        123456, 0x00C0FFEE, 250, 06675);

	Serial.printf("\nPrint pretty table\n");

	Serial.printf("LENGTH \t\tWIDTH \t\tHEIGHT \n");
	Serial.printf("%6d \t\t%5d \t\t%6d \n", 1, 2, 3);
	Serial.printf("%6d \t\t%5d \t\t%6d \n", 10, 20, 300);
	Serial.printf("%6.3f \t\t%5d \t\t%6.4f \n", 1.654, 200, 3.654);
	Serial.printf("%6.3f \t\t%5.2f \t\t%6.4f \n", PI, PI, PI);

	Serial.printf("\nTest crazy format specifiers on the same parameters\n");

	Serial.printf( "Display -99:%d, \t\t-3.0104:%f, \t'abc'=%s, \t\t0.75:%f, \t\t0.888888889:%f\n",
			-99, -3.01040, "abc", 3.0/4, 8.0/9);

	Serial.printf( "Display '   -99':%6d, \t-3.010:%.3f, \t\t'abc'=%5s, \t\t0.75000:%-+.5f, \t0.888888889:%#*f\n",
			-99, -3.01040, "abc", 3.0/4, 8.0/9);

	Serial.printf( "Display -99:%3d, \t\t-3.010:%.3f, \t\t'abc'=%#s, \t\t' 0.75':%5f, \t\t0.889:%.3f\n",
			-99, -3.01040, "abc", 3.0/4, 8.0/9);

	Serial.printf( "Display -99:%.3d, \t\t-3.0104000:%.7f, \t'abc'=%s, \t\t0.750:%.3f, \t\t' 0.889':%6.3f\n",
			-99, -3.01040, "abc", 3.0/4, 8.0/9);

	Serial.printf( "\nTest invalid specifiers and escaping of '%%': %j %w %%% %%%% % %zk \n");

	Serial.printf("\nShow limits:\n");

	Serial.printf("Int: %d .. %d \nUint limits 0 .. %u \nLong %ld .. %ld \n",
			INT_MIN + 1, INT_MAX, UINT_MAX, LONG_MIN + 1, LONG_MAX);

	Serial.printf("\n== TEST FINISH ==\n");
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
