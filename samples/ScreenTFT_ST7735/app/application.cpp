#include <SmingCore.h>
#include <Libraries/Adafruit_ST7735/Adafruit_ST7735.h>
#include <Libraries/Adafruit_GFX/BMPDraw.h>

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
#define TFT_SCLK 14
#define TFT_MOSI 13
#define TFT_RST 16
#define TFT_DC 0
#define TFT_CS 2

namespace
{
//Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void testdrawtext(const String& text, uint16_t color)
{
	tft.setCursor(0, 0);
	tft.setTextColor(color);
	tft.setTextWrap(true);
	tft.print(text);
}

void tftPrintTest1()
{
	tft.setTextWrap(false);
	tft.fillScreen(ST7735_BLACK);
	tft.setCursor(0, 30);
	tft.setTextColor(ST7735_RED);
	tft.setTextSize(1);
	tft.println("Hello Sming!");
	tft.setTextColor(ST7735_YELLOW);
	tft.setTextSize(2);
	tft.println("Hello Sming!");
	tft.setTextColor(ST7735_GREEN);
	tft.setTextSize(3);
	tft.println("Hello Sming!");
	tft.setTextColor(ST7735_BLUE);
	tft.setTextSize(4);
	tft.print(1234.567);
}

void tftPrintTest2()
{
	tft.setCursor(0, 0);
	tft.fillScreen(ST7735_BLACK);
	tft.setTextColor(ST7735_WHITE);
	tft.setTextSize(0);
	tft.println("Hello Sming!");
	tft.setTextSize(1);
	tft.setTextColor(ST7735_GREEN);
	tft.print(PI, 6);
	tft.println(" Want pi?");
	tft.println(" ");
	tft.print(8675309, HEX); // print 8,675,309 out in HEX!
	tft.println(" Print HEX!");
	tft.println(" ");
	tft.setTextColor(ST7735_WHITE);
	tft.println("Sketch has been");
	tft.println("running for: ");
	tft.setTextColor(ST7735_MAGENTA);
	tft.print(millis() / 1000);
	tft.setTextColor(ST7735_WHITE);
	tft.print(" seconds.");
}

void testlines(uint16_t color)
{
	tft.fillScreen(ST7735_BLACK);
	for(int16_t x = 0; x < tft.width(); x += 6) {
		tft.drawLine(0, 0, x, tft.height() - 1, color);
	}
	for(int16_t y = 0; y < tft.height(); y += 6) {
		tft.drawLine(0, 0, tft.width() - 1, y, color);
	}

	tft.fillScreen(ST7735_BLACK);
	for(int16_t x = 0; x < tft.width(); x += 6) {
		tft.drawLine(tft.width() - 1, 0, x, tft.height() - 1, color);
	}
	for(int16_t y = 0; y < tft.height(); y += 6) {
		tft.drawLine(tft.width() - 1, 0, 0, y, color);
	}

	// TODO: drawing from all 4 corners does kick in a watchdog reboot !
	/*

  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, tft.height()-1, x, 0, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, tft.height()-1, tft.width()-1, y, color);
  }

  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, x, 0, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, 0, y, color);
  }
  */
}

void testfastlines(uint16_t color1, uint16_t color2)
{
	tft.fillScreen(ST7735_BLACK);
	for(int16_t y = 0; y < tft.height(); y += 5) {
		tft.drawFastHLine(0, y, tft.width(), color1);
	}
	for(int16_t x = 0; x < tft.width(); x += 5) {
		tft.drawFastVLine(x, 0, tft.height(), color2);
	}
}

void testdrawrects(uint16_t color)
{
	tft.fillScreen(ST7735_BLACK);
	for(int16_t x = 0; x < tft.width(); x += 6) {
		tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x, color);
	}
}

void testfillrects(uint16_t color1, uint16_t color2)
{
	tft.fillScreen(ST7735_BLACK);
	for(int16_t x = tft.width() - 1; x > 6; x -= 6) {
		tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x, color1);
		tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x, color2);
	}
}

void testfillcircles(uint8_t radius, uint16_t color)
{
	for(int16_t x = radius; x < tft.width(); x += radius * 2) {
		for(int16_t y = radius; y < tft.height(); y += radius * 2) {
			tft.fillCircle(x, y, radius, color);
		}
	}
}

void testdrawcircles(uint8_t radius, uint16_t color)
{
	for(int16_t x = 0; x < tft.width() + radius; x += radius * 2) {
		for(int16_t y = 0; y < tft.height() + radius; y += radius * 2) {
			tft.drawCircle(x, y, radius, color);
		}
	}
}

void testroundrects()
{
	tft.fillScreen(ST7735_BLACK);
	int color = 100;
	int i;
	int t;
	for(t = 0; t <= 4; t += 1) {
		int x = 0;
		int y = 0;
		int w = tft.width() - 2;
		int h = tft.height() - 2;
		for(i = 0; i <= 16; i += 1) {
			tft.drawRoundRect(x, y, w, h, 5, color);
			x += 2;
			y += 3;
			w -= 4;
			h -= 6;
			color += 1100;
		}
		color += 100;
	}
}

void testtriangles()
{
	tft.fillScreen(ST7735_BLACK);
	int color = 0xF800;
	int t;
	int w = tft.width() / 2;
	int x = tft.height() - 1;
	int y = 0;
	int z = tft.width();
	for(t = 0; t <= 15; t += 1) {
		tft.drawTriangle(w, y, y, x, z, x, color);
		x -= 4;
		y += 4;
		z -= 4;
		color += 100;
	}
}

void mediabuttons()
{
	// play
	tft.fillScreen(ST7735_BLACK);
	tft.fillRoundRect(25, 10, 78, 60, 8, ST7735_WHITE);
	tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_RED);
	delay(500);
	// pause
	tft.fillRoundRect(25, 90, 78, 60, 8, ST7735_WHITE);
	tft.fillRoundRect(39, 98, 20, 45, 5, ST7735_GREEN);
	tft.fillRoundRect(69, 98, 20, 45, 5, ST7735_GREEN);
	delay(500);
	// play color
	tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_BLUE);
	delay(50);
	// pause color
	tft.fillRoundRect(39, 98, 20, 45, 5, ST7735_RED);
	tft.fillRoundRect(69, 98, 20, 45, 5, ST7735_RED);
	// play color
	tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_GREEN);
}

void screen13()
{
	tft.fillScreen(ST7735_BLACK);			// Clear display
	tft.setRotation(tft.getRotation() + 1); // Inc rotation 90 degrees
	for(uint8_t i = 0; i < 4; i++) {		// Draw 4 parrots
		bmpDraw(tft, "sming.bmp", tft.width() / 4 * i, tft.height() / 4 * i);
	}
}

void screen12()
{
	bmpDraw(tft, "sming.bmp", 0, 0);
}

void screen11()
{
	mediabuttons();
}

void screen10()
{
	testtriangles();
}

void screen9()
{
	testroundrects();
}

void screen8()
{
	tft.fillScreen(ST7735_BLACK);
	testfillcircles(10, ST7735_BLUE);
	testdrawcircles(10, ST7735_WHITE);
}

void screen7()
{
	testfillrects(ST7735_YELLOW, ST7735_MAGENTA);
}

void screen6()
{
	testdrawrects(ST7735_GREEN);
}

void screen5()
{
	// optimized lines
	testfastlines(ST7735_RED, ST7735_BLUE);
}

void screen4()
{
	// line draw test
	testlines(ST7735_YELLOW);
}

void screen3()
{
	tftPrintTest2();
}

void screen2()
{
	tftPrintTest1();
}

void screen1()
{
	// large block of text
	tft.fillScreen(ST7735_BLACK);
	DEFINE_FSTR_LOCAL(
		largeTextBlock,
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh "
		"tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed "
		"porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu "
		"hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ")
	testdrawtext(largeTextBlock, ST7735_WHITE);
}

void initialise()
{
	// Use this initializer if you're using a 1.8" TFT
	// tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

	// Use this initializer (uncomment) if you're using a 1.44" TFT
	tft.initR(INITR_144GREENTAB); // initialize a ST7735S chip, black tab

	tft.fillScreen(ST7735_BLACK);
}

#define SCREEN_FUNCTION_MAP(XX)                                                                                        \
	XX(initialise, "Initialise")                                                                                       \
	XX(screen1, "testdrawtext")                                                                                        \
	XX(screen2, "tftPrintTest1")                                                                                       \
	XX(screen3, "tftPrintTest2")                                                                                       \
	XX(screen4, "testlines")                                                                                           \
	XX(screen5, "testfastlines")                                                                                       \
	XX(screen6, "testdrawrects")                                                                                       \
	XX(screen7, "testfillrects")                                                                                       \
	XX(screen8, "testfillcircles")                                                                                     \
	XX(screen9, "testroundrects")                                                                                      \
	XX(screen10, "testtriangles")                                                                                      \
	XX(screen11, "mediabuttons")                                                                                       \
	XX(screen12, "bmpDraw")                                                                                            \
	XX(screen13, "bmpDraw rotation")

#define XX(function, title) DEFINE_FSTR(function##_title, #function ": " title)
SCREEN_FUNCTION_MAP(XX)
#undef XX

struct DemoScreen {
	InterruptCallback function;
	const FlashString& title;
};

const DemoScreen screenList[] PROGMEM{
#define XX(name, title) {name, name##_title},
	SCREEN_FUNCTION_MAP(XX)
#undef XX
};

SimpleTimer demoScreenTimer;
auto screen = std::begin(screenList);

void nextScreen()
{
	OneShotFastMs elapseTimer;
	screen->function();
	auto elapsed = elapseTimer.elapsedTime();

	Serial << screen->title << " in " << elapsed << "ms" << endl;

	++screen;
	if(screen != std::end(screenList)) {
		demoScreenTimer.startOnce();
		return;
	}

	Serial.println(_F("All screens displayed"));
}

} // namespace

void init()
{
#ifdef ARCH_HOST
	setDigitalHooks(nullptr);
#endif

	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

#ifndef DISABLE_WIFI
	//WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);
#endif

	Serial.println(_F("Initialise display"));
	demoScreenTimer.initializeMs<500>(nextScreen);
	nextScreen();
}
