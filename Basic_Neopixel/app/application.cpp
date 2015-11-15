#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <Adafruit_NeoPixel/Adafruit_NeoPixel.h>

// Which pin on the Esp8266 is connected to the NeoPixels?
#define PIN            5

// How many NeoPixels are attached to the Esp8266?
#define NUMPIXELS      8

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(50);
      WDT.alive();
  }

}

void init()
{


	// WIFI not needed for demo. So disabling WIFI.
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	strip.begin();

	colorWipe(strip.Color(15, 0, 0), 50); // Red
	colorWipe(strip.Color(0, 15, 0), 50); // Green
	colorWipe(strip.Color(0, 0, 15), 50); // Blue
}
