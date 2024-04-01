#include <SmingCore.h>
#include <Libraries/Adafruit_ILI9341/Adafruit_ILI9341.h>
#include <Libraries/Adafruit_GFX/BMPDraw.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
// See library for pinout
Adafruit_ILI9341 tft;

SimpleTimer guiTimer;

void basicBMP()
{
	debug_d("%s", __FUNCTION__);

	tft.fillScreen(ILI9341_BLACK);			// Clear display
	tft.setRotation(tft.getRotation() + 1); // Inc rotation 90 degrees
	for(uint8_t i = 0; i < 4; i++) {		// Draw 4 parrots
		bmpDraw(tft, "sming.bmp", i * tft.width() / 4, i * tft.height() / 4);
	}
}

void basicGui()
{
	debug_d("%s", __FUNCTION__);

	static int r;

	const int ara = 4;
	const int yerara = 15;
	const int u1 = 100;
	const int u2 = 320 - (u1 + ara);
	const int s1 = 0;
	const int s2 = (u1 + ara);

	const int g = 28;

	int p1 = 50;

	tft.setTextSize(1);

	tft.setRotation(1);

	tft.setTextSize(3);
	tft.fillRect(s1, 0, u1 * 2, 48, ILI9341_OLIVE);
	tft.setCursor(15, 15);
	tft.println("Sming");
	tft.setTextSize(2);
	tft.fillRect((u1 * 2) + ara, 0, 318 - (u1 * 2), 48, ILI9341_RED);
	for(auto a : {'a', 'b', 'c', 'd', 'e', 'f'}) {
		debug_d("%c: %u", a, r);
		tft.setTextColor(ILI9341_GREEN);
		tft.fillRect(s1, p1, u1, g, ILI9341_DARKCYAN);
		tft.setCursor(s1 + yerara, p1 + 6);
		tft.setTextColor(ILI9341_WHITE);
		tft.println(a);
		tft.fillRect(s2, p1, u2, g, ILI9341_DARKCYAN);
		tft.setCursor(s2 + yerara, p1 + 6);
		tft.println(r);
		p1 += g + 4;
	}
	p1 = 50;
	r++;

	if(r >= 5) {
		guiTimer.setCallback(basicBMP);
	}

	guiTimer.startOnce();
}

void demo()
{
	Serial.println(_F("Display start"));

	// text display tests
	tft.begin();
	tft.fillScreen(0);
	tft.setRotation(1);
	tft.setTextSize(2);

	tft.setTextColor(ILI9341_GREEN);
	tft.setCursor(0, 0);
	tft.setCursor(60, 60);
	tft.println(_F("Sming  Framework"));
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // text
	tft.setCursor(60, 75);
	tft.println("              v1.1");
	tft.setTextColor(ILI9341_CYAN);
	tft.setCursor(60, 90);
	tft.println(_F("ili9340-40C-41 "));
	tft.setCursor(60, 125);
	tft.println(_F("M.Bozkurt"));

	guiTimer.initializeMs<2000>([]() {
		tft.fillScreen(0);
		guiTimer.initializeMs<1000>(basicGui).startOnce();
	});
	guiTimer.startOnce();
}

} // namespace

void init()
{
#ifdef ARCH_HOST
	setDigitalHooks(nullptr);
#endif

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

#ifndef DISABLE_WIFI
	//WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);
#endif

	spiffs_mount();
	Serial.println(_F("FileSystem mounted."));

	demo();
}
