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

        // Note that the leading '#' (as in %#x) is currently ignored
	Serial.printf("\nShow a decimal %d, a hex %#x, an unsigned %u, an octal %o.\n",
	        123456, 0x00C0FFEE, 250, 06675);
	Serial.printf("Field width is supported: [%8d], with optional zero-padding: 0x%08x\n",
	        123456, 0xC0FFEE);

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

	debugf("\n(DEBUG) message printed on UART0\n");

	/**
	 * Serial1 uses UART1, TX pin is GPIO2.
	 * UART1 can not be used to receive data because normally
	 * it's RX pin is occupied for flash chip connection.
	 *
	 * If you have a spare serial to USB converter do the following to see the
	 * messages printed on UART1:
	 * - connect converter GND to esp8266 GND
	 * - connect converter RX to esp8266 GPIO2
	 */
	HardwareSerial Serial1(UART1);
	Serial1.begin(SERIAL_BAUD_RATE);

	/*
	 * The line below redirect debug output to UART1
	 */
	Serial1.systemDebugOutput(true);
	Serial1.printf("====Debug Information=====\n");

	debugf("(DEBUG) message printed on UART1\n"); // You should see the debug message in UART1 only.

	procTimer.initializeMs(2000, sayHello).start();

	testPrintf();

	/// Reading callback example:
	//  * Option 1
	//	Set Serial Callback to global routine:
	//	   Serial.setCallback(onDataCallback);
	// If you want to test local echo set the following callback
	//	   Serial.setCallback(echoCallback);

	// 	* Option 2
	//  Instantiate hwsDelegateDemo which includes Serial Delegate class
	delegateDemoClass.begin();
}
