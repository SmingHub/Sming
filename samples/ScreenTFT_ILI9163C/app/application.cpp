#include <SmingCore.h>
#include <Libraries/TFT_ILI9163C/TFT_ILI9163C.h>

namespace
{
/*
 * LED      (BACKLIGHT)   3.3v
 * SCK      (SCLK)        GPIO14
 * SDA      (MOSI)        GPIO13
 * A0       (DC)          GPIO0
 * RESET    (RESET)       3.3v
 * CS       (CS)          GPIO2
 * GND      (GND)         GND
 * VCC      (VCC)         3.3v
 */
TFT_ILI9163C tft(2, 0);
SimpleTimer timer;

void demo()
{
	Serial.println(_F("Display start"));
	tft.begin();
	tft.setRotation(2); // try yourself
	tft.fillScreen();
	tft.fillRect(10, 20, 100, 120, YELLOW);

	timer.initializeMs<1000>([]() {
		// text display tests
		tft.fillScreen();
		tft.setTextSize(1);
		tft.setTextColor(GREEN);
		tft.setCursor(0, 0);
		tft.println(_F("Sming Framework"));
		tft.setTextColor(BLACK, WHITE); // 'inverted' text
		tft.setCursor(104, 7);
		tft.println("v1.0");
		tft.setTextColor(WHITE);
		tft.println(_F("Let's do smart things"));
		tft.setTextSize(3);
		tft.setTextColor(BLUE);
		tft.print("IoT");
	});
	timer.startOnce();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	demo();
}
