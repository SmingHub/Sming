#include <SmingCore.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

/*
 * Hardware SPI mode:
 * GND      (GND)         GND
 * VCC      (VCC)         3.3v
 * D0       (CLK)         GPIO14
 * D1       (MOSI)        GPIO13
 * RES      (RESET)       GPIO16
 * DC       (DC)          GPIO0
 * CS       (CS)          GPIO2
 */

// to use other kind of display supported by library, eg. SH1106 1.3in:
/*
#undef SSD1306_128_64  // this one is default, 0.96in SSD1306
#define SH1106_128_64  // larger one
*/

// For spi oled module
// Adafruit_SSD1306 display(0, 16, 2);

//* For I2C mode:
// Default I2C pins 0 (SCL) and 2 (SDA). Pin 4 - optional reset
Adafruit_SSD1306 display(-1); // reset Pin required but later ignored if set to False

Timer DemoTimer;

void Demo2()
{
	Serial.println("Display: some text");
	display.clearDisplay();
	// text display tests
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	display.println("Sming Framework");
	display.setTextColor(BLACK, WHITE); // 'inverted' text
	display.setCursor(104, 7);
	display.println("v1.0");
	//----
	display.setTextColor(WHITE);
	display.println("Let's do smart things");
	display.setTextSize(3);
	display.print("IoT");
	display.display();
	DemoTimer.stop(); // Finish demo
}

void Demo1()
{
	Serial.println("Display: circle");
	// Clear the buffer.
	display.clearDisplay();
	// draw a circle, 10 pixel radius
	display.fillCircle(display.width() / 2, display.height() / 2, 10, WHITE);
	display.display();
	DemoTimer.stop();
	DemoTimer.initializeMs(2000, Demo2).start();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.println("Display: start");

	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
	// initialize with the I2C addr 0x3c (for the 128x64)
	// bool:reset set to TRUE or FALSE depending on your display
	display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
	display.display();
	DemoTimer.initializeMs(2000, Demo1).start();
}
