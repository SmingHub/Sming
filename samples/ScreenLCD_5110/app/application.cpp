#include <SmingCore.h>
#include <Libraries/Adafruit_PCD8544/Adafruit_PCD8544.h>

// GPIO13/D7 - Serial clock out (SCLK)
// GPIO12/D6 - Serial data out (DIN)
// GPIO14/D5 - Data/Command select (D/C)
// GPIO5/D1 - LCD chip select (CS)
// GPIO4/D2 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(13, 12, 14, 5, 4);

void displayTest()
{
	display.begin();
	display.setContrast(10);
	display.display(); // show splashscreen
	delay(2000);
	display.clearDisplay(); // no changes will be visible until display() is called
	display.setRotation(4); // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
	display.setTextSize(1);
	display.setTextColor(BLACK);
	display.setCursor(0, 0);
	display.println("Sming");
	display.setTextSize(2);
	display.println("Example");
	display.display();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	displayTest();
}
