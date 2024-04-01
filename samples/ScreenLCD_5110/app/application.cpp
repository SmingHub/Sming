#include <SmingCore.h>
#include <Libraries/Adafruit_PCD8544/Adafruit_PCD8544.h>

namespace
{
// GPIO13/D7 - Serial clock out (SCLK)
// GPIO12/D6 - Serial data out (DIN)
// GPIO14/D5 - Data/Command select (D/C)
// GPIO5/D1 - LCD chip select (CS)
// GPIO4/D2 - LCD reset (RST)
Adafruit_PCD8544 display(13, 12, 14, 5, 4);
SimpleTimer timer;

void displayTest()
{
	display.begin();
	display.setContrast(10);
	display.display(); // show splashscreen
	debug_d("Show splash...");

	timer.initializeMs<2000>([]() {
		debug_d("Update screen.");
		display.clearDisplay(); // no changes will be visible until display() is called
		display.setRotation(4); // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
		display.setTextSize(1);
		display.setTextColor(BLACK);
		display.setCursor(0, 0);
		display.println("Sming");
		display.setTextSize(2);
		display.println("Example");
		display.display();
	});
	timer.startOnce();
}

} // namespace

void init()
{
#ifdef ARCH_HOST
	setDigitalHooks(nullptr);
#endif

	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	displayTest();
}
