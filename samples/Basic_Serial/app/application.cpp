/*
 * application.cpp
 */

#include "SerialReadingDelegateDemo.h"
#include "SerialTransmitDemo.h"

DEFINE_FSTR(testFlashData, "The following are the graphical (non-control) characters defined by\r\n"
						   "ISO 8859-1 (1987).  Descriptions in words aren't all that helpful,\r\n"
						   "but they're the best we can do in text.  A graphics file illustrating\r\n"
						   "the character set should be available from the same archive as this\r\n"
						   "file.\r\n"
						   "\r\n"
						   "Hex Description                 Hex Description\r\n"
						   "\r\n"
						   "20  SPACE\r\n"
						   "21  EXCLAMATION MARK            A1  INVERTED EXCLAMATION MARK\r\n"
						   "22  QUOTATION MARK              A2  CENT SIGN\r\n"
						   "23  NUMBER SIGN                 A3  POUND SIGN\r\n"
						   "24  DOLLAR SIGN                 A4  CURRENCY SIGN\r\n"
						   "25  PERCENT SIGN                A5  YEN SIGN\r\n"
						   "26  AMPERSAND                   A6  BROKEN BAR\r\n"
						   "27  APOSTROPHE                  A7  SECTION SIGN\r\n"
						   "28  LEFT PARENTHESIS            A8  DIAERESIS\r\n"
						   "29  RIGHT PARENTHESIS           A9  COPYRIGHT SIGN\r\n"
						   "2A  ASTERISK                    AA  FEMININE ORDINAL INDICATOR\r\n"
						   "2B  PLUS SIGN                   AB  LEFT-POINTING DOUBLE ANGLE QUOTATION MARK\r\n"
						   "2C  COMMA                       AC  NOT SIGN\r\n"
						   "2D  HYPHEN-MINUS                AD  SOFT HYPHEN\r\n"
						   "2E  FULL STOP                   AE  REGISTERED SIGN\r\n"
						   "2F  SOLIDUS                     AF  OVERLINE\r\n"
						   "30  DIGIT ZERO                  B0  DEGREE SIGN\r\n"
						   "31  DIGIT ONE                   B1  PLUS-MINUS SIGN\r\n"
						   "32  DIGIT TWO                   B2  SUPERSCRIPT TWO\r\n"
						   "33  DIGIT THREE                 B3  SUPERSCRIPT THREE\r\n"
						   "34  DIGIT FOUR                  B4  ACUTE ACCENT\r\n"
						   "35  DIGIT FIVE                  B5  MICRO SIGN\r\n"
						   "36  DIGIT SIX                   B6  PILCROW SIGN\r\n"
						   "37  DIGIT SEVEN                 B7  MIDDLE DOT\r\n"
						   "38  DIGIT EIGHT                 B8  CEDILLA\r\n"
						   "39  DIGIT NINE                  B9  SUPERSCRIPT ONE\r\n"
						   "3A  COLON                       BA  MASCULINE ORDINAL INDICATOR\r\n"
						   "3B  SEMICOLON                   BB  RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK\r\n"
						   "3C  LESS-THAN SIGN              BC  VULGAR FRACTION ONE QUARTER\r\n"
						   "3D  EQUALS SIGN                 BD  VULGAR FRACTION ONE HALF\r\n"
						   "3E  GREATER-THAN SIGN           BE  VULGAR FRACTION THREE QUARTERS\r\n"
						   "3F  QUESTION MARK               BF  INVERTED QUESTION MARK\r\n"
						   "40  COMMERCIAL AT               C0  CAPITAL LETTER A WITH GRAVE\r\n"
						   "41  CAPITAL LETTER A            C1  CAPITAL LETTER A WITH ACUTE\r\n"
						   "42  CAPITAL LETTER B            C2  CAPITAL LETTER A WITH CIRCUMFLEX\r\n"
						   "43  CAPITAL LETTER C            C3  CAPITAL LETTER A WITH TILDE\r\n"
						   "44  CAPITAL LETTER D            C4  CAPITAL LETTER A WITH DIAERESIS\r\n"
						   "45  CAPITAL LETTER E            C5  CAPITAL LETTER A WITH RING ABOVE\r\n"
						   "46  CAPITAL LETTER F            C6  CAPITAL LETTER AE\r\n"
						   "47  CAPITAL LETTER G            C7  CAPITAL LETTER C WITH CEDILLA\r\n"
						   "48  CAPITAL LETTER H            C8  CAPITAL LETTER E WITH GRAVE\r\n"
						   "49  CAPITAL LETTER I            C9  CAPITAL LETTER E WITH ACUTE\r\n"
						   "4A  CAPITAL LETTER J            CA  CAPITAL LETTER E WITH CIRCUMFLEX\r\n"
						   "4B  CAPITAL LETTER K            CB  CAPITAL LETTER E WITH DIAERESIS\r\n"
						   "4C  CAPITAL LETTER L            CC  CAPITAL LETTER I WITH GRAVE\r\n"
						   "4D  CAPITAL LETTER M            CD  CAPITAL LETTER I WITH ACUTE\r\n"
						   "4E  CAPITAL LETTER N            CE  CAPITAL LETTER I WITH CIRCUMFLEX\r\n"
						   "4F  CAPITAL LETTER O            CF  CAPITAL LETTER I WITH DIAERESIS\r\n"
						   "50  CAPITAL LETTER P            D0  CAPITAL LETTER ETH (Icelandic)\r\n"
						   "51  CAPITAL LETTER Q            D1  CAPITAL LETTER N WITH TILDE\r\n"
						   "52  CAPITAL LETTER R            D2  CAPITAL LETTER O WITH GRAVE\r\n"
						   "53  CAPITAL LETTER S            D3  CAPITAL LETTER O WITH ACUTE\r\n"
						   "54  CAPITAL LETTER T            D4  CAPITAL LETTER O WITH CIRCUMFLEX\r\n"
						   "55  CAPITAL LETTER U            D5  CAPITAL LETTER O WITH TILDE\r\n"
						   "56  CAPITAL LETTER V            D6  CAPITAL LETTER O WITH DIAERESIS\r\n"
						   "57  CAPITAL LETTER W            D7  MULTIPLICATION SIGN\r\n"
						   "58  CAPITAL LETTER X            D8  CAPITAL LETTER O WITH STROKE\r\n"
						   "59  CAPITAL LETTER Y            D9  CAPITAL LETTER U WITH GRAVE\r\n"
						   "5A  CAPITAL LETTER Z            DA  CAPITAL LETTER U WITH ACUTE\r\n"
						   "5B  LEFT SQUARE BRACKET         DB  CAPITAL LETTER U WITH CIRCUMFLEX\r\n"
						   "5C  REVERSE SOLIDUS             DC  CAPITAL LETTER U WITH DIAERESIS\r\n"
						   "5D  RIGHT SQUARE BRACKET        DD  CAPITAL LETTER Y WITH ACUTE\r\n"
						   "5E  CIRCUMFLEX ACCENT           DE  CAPITAL LETTER THORN (Icelandic)\r\n"
						   "5F  LOW LINE                    DF  SMALL LETTER SHARP S (German)\r\n"
						   "60  GRAVE ACCENT                E0  SMALL LETTER A WITH GRAVE\r\n"
						   "61  SMALL LETTER A              E1  SMALL LETTER A WITH ACUTE\r\n"
						   "62  SMALL LETTER B              E2  SMALL LETTER A WITH CIRCUMFLEX\r\n"
						   "63  SMALL LETTER C              E3  SMALL LETTER A WITH TILDE\r\n"
						   "64  SMALL LETTER D              E4  SMALL LETTER A WITH DIAERESIS\r\n"
						   "65  SMALL LETTER E              E5  SMALL LETTER A WITH RING ABOVE\r\n"
						   "66  SMALL LETTER F              E6  SMALL LETTER AE\r\n"
						   "67  SMALL LETTER G              E7  SMALL LETTER C WITH CEDILLA\r\n"
						   "68  SMALL LETTER H              E8  SMALL LETTER E WITH GRAVE\r\n"
						   "69  SMALL LETTER I              E9  SMALL LETTER E WITH ACUTE\r\n"
						   "6A  SMALL LETTER J              EA  SMALL LETTER E WITH CIRCUMFLEX\r\n"
						   "6B  SMALL LETTER K              EB  SMALL LETTER E WITH DIAERESIS\r\n"
						   "6C  SMALL LETTER L              EC  SMALL LETTER I WITH GRAVE\r\n"
						   "6D  SMALL LETTER M              ED  SMALL LETTER I WITH ACUTE\r\n"
						   "6E  SMALL LETTER N              EE  SMALL LETTER I WITH CIRCUMFLEX\r\n"
						   "6F  SMALL LETTER O              EF  SMALL LETTER I WITH DIAERESIS\r\n"
						   "70  SMALL LETTER P              F0  SMALL LETTER ETH (Icelandic)\r\n"
						   "71  SMALL LETTER Q              F1  SMALL LETTER N WITH TILDE\r\n"
						   "72  SMALL LETTER R              F2  SMALL LETTER O WITH GRAVE\r\n"
						   "73  SMALL LETTER S              F3  SMALL LETTER O WITH ACUTE\r\n"
						   "74  SMALL LETTER T              F4  SMALL LETTER O WITH CIRCUMFLEX\r\n"
						   "75  SMALL LETTER U              F5  SMALL LETTER O WITH TILDE\r\n"
						   "76  SMALL LETTER V              F6  SMALL LETTER O WITH DIAERESIS\r\n"
						   "77  SMALL LETTER W              F7  DIVISION SIGN\r\n"
						   "78  SMALL LETTER X              F8  SMALL LETTER O WITH STROKE\r\n"
						   "79  SMALL LETTER Y              F9  SMALL LETTER U WITH GRAVE\r\n"
						   "7A  SMALL LETTER Z              FA  SMALL LETTER U WITH ACUTE\r\n"
						   "7B  LEFT CURLY BRACKET          FB  SMALL LETTER U WITH CIRCUMFLEX\r\n"
						   "7C  VERTICAL LINE               FC  SMALL LETTER U WITH DIAERESIS\r\n"
						   "7D  RIGHT CURLY BRACKET         FD  SMALL LETTER Y WITH ACUTE\r\n"
						   "7E  TILDE                       FE  SMALL LETTER THORN (Icelandic)\r\n"
						   "                                FF  SMALL LETTER Y WITH DIAERESIS\r\n"
						   "\r\n"
						   "END OF TEXT\r\n");

Timer procTimer;
SerialReadingDelegateDemo delegateDemoClass;
int helloCounter = 0;

/**
 * Serial1 uses UART1, TX pin is GPIO2.
 * UART1 can not be used to receive data because normally
 * it's RX pin is occupied for flash chip connection.
 *
 * If you have a spare serial to USB converter do the following to see the
 * messages printed on UART1:
 * - connect converter GND to esp8266 GND
 * - connect converter RX to esp8266 GPIO2
 *
 * We demonstrate sending large amounts of data, by default sending it to the debug port. Running both together
 * demonstrates how fast and fluid Sming can be.
 */
HardwareSerial Serial1(UART_ID_1);

void sayHello()
{
	Serial.print(_F("Hello Sming! Let's do smart things."));
	Serial.print(_F(" Time : "));
	Serial.println(micros());
	Serial.println();

	Serial.printf(_F("This is Hello message %d \r\r\n"), ++helloCounter);
}

void testPrintf()
{
	Serial.print(_F("\r\n== PRINTF TEST START ==\r\n"));

	Serial.print(_F("\r\nFloat numbers display test: \r\n"));

	Serial.printf("Pi with 2 decimals: %.2f and with 4 decimals: %.4f \r\n", PI, PI);
	Serial.printf("Pi without specifying precision(default 9): %f\r\n", PI);
	Serial.printf("A number with leading fraction zeroes like 1.050250=%f\r\n", 1.050250);

	Serial.print(_F(
		"\r\nTest crazy line lengths: wait for it, wait for it, wait for it, wait for it, "
		"wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, "
		"wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, "
		"wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, wait for it, got "
		"to the end :)\r\n"));

	// Note that the leading '#' (as in %#x) is currently ignored
	Serial.printf("\r\nShow a decimal %d, a hex %#x, an unsigned %u, an octal %o.\r\n", 123456, 0x00C0FFEE, 250, 06675);
	Serial.printf("Field width is supported: [%8d], with optional zero-padding: 0x%08x\r\n", 123456, 0xC0FFEE);

	Serial.print(_F("\r\nPrint pretty table\r\n"));

	Serial.print(_F("LENGTH \t\tWIDTH \t\tHEIGHT \r\n"));
	Serial.printf("%6d \t\t%5d \t\t%6d \r\n", 1, 2, 3);
	Serial.printf("%6d \t\t%5d \t\t%6d \r\n", 10, 20, 300);
	Serial.printf("%6.3f \t\t%5d \t\t%6.4f \r\n", 1.654, 200, 3.654);
	Serial.printf("%6.3f \t\t%5.2f \t\t%6.4f \r\n", PI, PI, PI);

	Serial.print(_F("\r\nTest crazy format specifiers on the same parameters\r\n"));

	Serial.printf("Display -99:%d, \t\t-3.0104:%f, \t'abc'=%s, \t\t0.75:%f, \t\t0.888888889:%f\r\n", -99, -3.01040,
				  "abc", 3.0 / 4, 8.0 / 9);

	Serial.printf("Display '   -99':%6d, \t-3.010:%.3f, \t\t'abc'=%5s, \t\t0.75000:%-+.5f, \t0.888888889:%#*f\r\n", -99,
				  -3.01040, "abc", 3.0 / 4, 8.0 / 9);

	Serial.printf("Display -99:%3d, \t\t-3.010:%.3f, \t\t'abc'=%#s, \t\t' 0.75':%5f, \t\t0.889:%.3f\r\n", -99, -3.01040,
				  "abc", 3.0 / 4, 8.0 / 9);

	Serial.printf("Display -99:%.3d, \t\t-3.0104000:%.7f, \t'abc'=%s, \t\t0.750:%.3f, \t\t' 0.889':%6.3f\r\n", -99,
				  -3.01040, "abc", 3.0 / 4, 8.0 / 9);

	Serial.print(_F("\r\nTest invalid specifiers and escaping of '%%': %j %w %%% %%%% % %zk \r\n"));

	Serial.print(_F("\r\nShow limits:\r\n"));

	Serial.printf("Int: %d .. %d \r\nUint limits 0 .. %u \r\nLong %ld .. %ld \r\n", INT_MIN + 1, INT_MAX, UINT_MAX,
				  LONG_MIN + 1, LONG_MAX);

	Serial.println(_F("\r\n== TEST FINISH ==\r\n"));
}

// SerialReadingDelegateDemo calls this function when a command line has been entered
void handleCommand(const String& command)
{
	if(command.equalsIgnoreCase(_F("cat"))) {
		String filename = F("Readme.md");
		FileStream* fileStream = new FileStream;
		if(fileStream && fileStream->open(filename, eFO_ReadOnly)) {
			Serial.printf(_F("Sending \"%s\" (%u bytes)\r\n"), filename.c_str(), fileStream->available());
			auto demo = new SerialTransmitDemo(Serial1, fileStream);
			demo->begin();
		} else {
			Serial.printf(_F("Failed to open file \"%s\"\r\n"), filename.c_str());
			delete fileStream;
		}
	} else if(command.equalsIgnoreCase(_F("text"))) {
		Serial.printf(_F("Sending flash data, %u bytes\r\n"), testFlashData.length());
		auto demo = new SerialTransmitDemo(Serial, new FlashMemoryStream(testFlashData));
		demo->begin();
	} else {
		Serial.printf(_F("I don't know what \"%s\" means! Try typing: cat\r\n"), command.c_str());
	}
}

void init()
{
	/*
	 * If the transmit buffer is full then default behaviour when writing is to sit in a loop waiting for space.
	 * This slows the application significantly, and is generally a bad thing.
	 *
	 * There are several solutions to this:
	 *
	 * 	1. Set a larger transmit buffer size
	 * 	2. Turn off waiting on transmit buffer full (TxWait, on by default)
	 *
	 * If you're using the serial port for debugging, and don't want the application to stall then set a large transmit
	 * buffer size and turn TxWait off. Surplus characters won't be written, and you can detect this by checking the
	 * return value from HardwareSerial::write, etc. if it's important.
	 *
	 * The optimal way to output data to the serial port is to break up large transfers into chunks, and use a callback
	 * to refill the buffer when it becomes empty. This demo shows how that can be done for outputting a file.
	 */
	Serial.setTxBufferSize(2048);
	// Serial.setTxWait(false);

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	// There's a large file on SPIFFS we'll be accessing later on
	spiffs_mount();

	/*There are four debug levels: debug=3, info=2, warn=1, error=0
	 * You can set the debug level by making with DEBUG_VERBOSE_LEVEL
	 * set to the desired level (0-3). Ex make rebuild DEBUG_VERBOSE_LEVEL=2 will
	 * show only info messages and above, will not show debug messages
	 * (they will be removed from the build at compile time, saving flash space)
	 *
	 * Functions debugf, debug_d, debug_i, debug_w, and debug_e store the format string
	 * in flash so that the RAM is freed for more important information.
	 *
	 * Another useful feature is printing the filename and line number of every debug line
	 * This will require extra space on flash and can be enabled
	 * using make parameter PRINT_FILENAME_AND_LINE=1*/

	debug_d("(DEBUG) message printed on UART0");

	debug_i("(INFO) message printed on UART0");

	debug_w("(WARNING) message printed on UART0");

	debug_e("(ERROR) message printed on UART0");

	/*debugf is equivalent to debug_i*/
	debugf("(INFO) message printed with debugf on UART0");

	/*
	 * Notice: The line below disables the debugf output on all UARTs.
	 */
	Serial.systemDebugOutput(false);

	debugf("(DEBUG) don't print me at all");

	/*
	 * The debugf output is redirected to UART0
	 * together with the system debug messages.
	 */
	Serial.systemDebugOutput(true);
	delay(200);

	debugf("(DEBUG) print me again on UART0");

	// Initialise and prepare the second (debug) serial port
	Serial1.begin(SERIAL_BAUD_RATE);

	/*
	 * The line below redirect debug output to UART1
	 */
	Serial1.systemDebugOutput(true);
	Serial1.println(_F("====Debug Information====="));

	debugf("(DEBUG) message printed on UART1"); // You should see the debug message in UART1 only.

	procTimer.initializeMs(2000, sayHello).start();

	testPrintf();

	/// Reading callback example:
	//  * Option 1
	//	Set Serial Callback to global routine:
	//	   Serial.onDataReceived(onDataCallback);
	// If you want to test local echo set the following callback
	//	   Serial.onDataReceived(echoCallback);

	// 	* Option 2
	//  Instantiate hwsDelegateDemo which includes Serial Delegate class
	delegateDemoClass.begin(Serial);

	delegateDemoClass.onCommand(handleCommand);
}
