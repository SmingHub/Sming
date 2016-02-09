#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>


#ifndef WIFI_SSID
	#define WIFI_SSID "XXX" 			// Put you SSID and Password here
	#define WIFI_PWD "XXX"
#endif


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
// For spi oled module
// Adafruit_SSD1306 display(0, 16, 2);

//* For I2C mode:
// Default I2C pins 0 (SCL) and 2 (SDA). Pin 4 - optional reset
Adafruit_SSD1306 display(-1); // reset Pin required but later ignored if set to False


Timer DemoTimer;

// Will be called when WiFi station was connected to AP
void connect_Ok()
{
	Serial.print("WiFi : I'm CONNECTED - ");
	Serial.println(WifiStation.getIP().toString());

	//You can put here other job like web,tcp, send other info to display etc.
}

// Will be called when WiFi station timeout was reached
void connect_Fail()
{
	Serial.println("WiFi: I'm NOT CONNECTED!");
	WifiStation.waitConnection(connect_Ok, 10, connect_Fail); // Repeat and check again
}


void Demo2()
{
	Serial.println("Display: some text");
	display.clearDisplay();
	// text display tests
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
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
	DemoTimer.stop();      // Finish demo
}

void Demo1()
{
        Serial.println("Display: circle");
	// Clear the buffer.
	display.clearDisplay();
	// draw a circle, 10 pixel radius
	display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
	display.display();
	DemoTimer.stop();
	DemoTimer.initializeMs(2000, Demo2).start();	
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial
	
	// Wifi could be used eg. for display info from internet
	// could be also dissabled if no needed

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);
	WifiStation.waitConnection(connect_Ok, 20, connect_Fail);

	Serial.println("Display: start");

	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)`
	// initialize with the I2C addr 0x3D (for the 128x64)
	// bool:reset set to TRUE or FALSE depending on you display
	display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, FALSE);
	// display.begin(SSD1306_SWITCHCAPVCC);
	display.display();
	DemoTimer.initializeMs(2000, Demo1).start();
}	
