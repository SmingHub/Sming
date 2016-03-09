#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Adafruit_PCD8544/Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
	// pin 7 - Serial clock out (SCLK)
	// pin 6 - Serial data out (DIN)
	// pin 5 - Data/Command select (D/C)
	// pin 4 - LCD chip select (CS)
	// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);
void displayTest()
{
	display.begin();
	  // init done

	  // you can change the contrast around to adapt the display
	  // for the best viewing!
	  display.setContrast(10);

	  display.display(); // show splashscreen
	  delay(200);
	  display.clearDisplay();   // clears the screen and buffer

	  display.setRotation(4);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
	    display.setTextSize(1);
	    display.setTextColor(BLACK);
	    display.setCursor(0,0);
	    display.println("Sming");
	    display.setTextSize(2);
	    display.println("Example!");
	    display.display();
}
void init()
{
	Serial.begin(SERIAL_BAUDRATE_APP); // 115200 by default, change it in Makefile-user.mk
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(false);
	WifiAccessPoint.enable(true);

	displayTest();
}

