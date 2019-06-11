#include <SmingCore.h>
#include <Libraries/Adafruit_ILI9341/Adafruit_ILI9341.h>
#include <Libraries/Adafruit_GFX/BMPDraw.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

/*
Pinout:
MISO GPIO12
MOSI GPIO13
CLK GPIO14
CS GPIO15
DC GPIO5
RST GPIO4
*/

Adafruit_ILI9341 tft;

Timer guiTimer;

int r = 0;

int ara = 4, yerara = 15;
int u1 = 100;
int u2 = 320 - (u1 + ara);
int s1 = 0;
int s2 = (u1 + ara);
int p1 = 50;

int g = 28;
int y = 90;
int satir = 6;

String lists[] = {"a", "b", "c", "d", "e", "f"};

void basicBPM()
{
	tft.fillScreen(ILI9341_BLACK);			// Clear display
	tft.setRotation(tft.getRotation() + 1); // Inc rotation 90 degrees
	for(uint8_t i = 0; i < 4; i++)			// Draw 4 parrots
		bmpDraw(tft, "sming.bmp", tft.width() / 4 * i, tft.height() / 4 * i);
}

void basicGui()
{
	tft.setTextSize(1);

	tft.setRotation(1);

	tft.setTextSize(3);
	tft.fillRect(s1, 0, u1 * 2, 48, ILI9341_OLIVE);
	tft.setCursor(15, 15);
	tft.println("Sming");
	tft.setTextSize(2);
	tft.fillRect((u1 * 2) + ara, 0, 318 - (u1 * 2), 48, ILI9341_RED);
	for(int a = 0; a < satir; a++) {
		tft.setTextColor(ILI9341_GREEN);
		tft.fillRect(s1, p1, u1, g, ILI9341_DARKCYAN);
		tft.setCursor(s1 + yerara, p1 + 6);
		tft.setTextColor(ILI9341_WHITE);
		tft.println(lists[a]);
		tft.fillRect(s2, p1, u2, g, ILI9341_DARKCYAN);
		tft.setCursor(s2 + yerara, p1 + 6);
		tft.println(r);
		p1 += g + 4;
	}
	p1 = 50;
	r++;
	guiTimer.initializeMs(1000, basicBPM).start(false);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial
	//WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	spiffs_mount();
	Serial.println("FileSystem mounted.");

	//	delay(2000);
	Serial.println("Display start");

	// text display tests
	tft.begin();
	tft.fillScreen(0);
	tft.setRotation(1);
	tft.setTextSize(2);

	tft.setTextColor(ILI9341_GREEN);
	tft.setCursor(0, 0);
	tft.setCursor(60, 60);
	tft.println("Sming  Framework");
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // text
	tft.setCursor(60, 75);
	tft.println("              v1.1");
	tft.setTextColor(ILI9341_CYAN);
	tft.setCursor(60, 90);
	tft.println("ili9340-40C-41 ");
	tft.setCursor(60, 125);
	tft.println("M.Bozkurt");
	delay(2000);
	tft.fillScreen(0);
	guiTimer.initializeMs(1000, basicGui).start(false);
	//runTest();
}
