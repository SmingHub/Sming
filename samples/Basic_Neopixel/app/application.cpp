#include <SmingCore.h>

#include <Libraries/Adafruit_NeoPixel/Adafruit_NeoPixel.h>

#ifndef WIFI_SSID
#define WIFI_SSID "XXX" // Put your SSID and password here
#define WIFI_PWD "XXX"
#endif

// Which pin on the Esp8266 is connected to the NeoPixels?
#define PIN 15

// How many NeoPixels are attached to the Esp8266?
#define NUMPIXELS 16

namespace
{
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void startDemo();

SimpleTimer timer;
uint32_t stripColor;
uint8_t stripDemoType; // Defined by demoData below
uint8_t chaseCycle;

enum ColorMap {
	CLR_BLACK = 0x000000,
	CLR_RED = 0xff0000,
	CLR_GREEN = 0x00ff00,
	CLR_BLUE = 0x0000ff,
	CLR_DIM_WHITE = 0x7f7f7f,
	CLR_DIM_RED = 0x7f0000,
	CLR_DIM_BLUE = 0x00007f,
};

struct DemoData {
	uint32_t color;
	uint16_t stepMilliseconds;
	uint8_t chase;
};

const DemoData demoData[]{
	{CLR_BLACK, 50, 0},		 // 0
	{CLR_RED, 50, 0},		 // 1
	{CLR_GREEN, 100, 0},	 // 2
	{CLR_BLUE, 150, 0},		 // 3
	{CLR_BLACK, 50, 0},		 // 4
	{CLR_DIM_WHITE, 50, 10}, // 5
	{CLR_DIM_RED, 60, 15},   // 6
	{CLR_DIM_BLUE, 70, 20},  // 7
};

//
// Fill the dots one after the other with a color
// Timer callback
//
void colorWipe()
{
	static uint16_t stripNumber;

	if(stripNumber >= strip.numPixels()) {
		stripNumber = 0;
		// start next demo after 2 seconds
		stripDemoType++;
		timer.initializeMs<2000>(startDemo).startOnce();
		return;
	}

	strip.setPixelColor(stripNumber, stripColor);
	strip.show();
	++stripNumber;
}

//
// Theatre-style crawling lights
// Timer callback
//
void theatreChase()
{
	static uint8_t theatreChaseQ;

	if(chaseCycle <= 0) {
		assert(theatreChaseQ == 0);
		// start another demo after 2 seconds
		stripDemoType++;
		timer.initializeMs<2000>(startDemo).startOnce();
		return;
	}

	const uint8_t offsetList[]{3, 0, 1, 2};
	int b = offsetList[theatreChaseQ];

	// Turn every third pixel off
	for(unsigned i = 0; i < strip.numPixels(); i += 4) {
		strip.setPixelColor(i + b, 0);
		strip.setPixelColor(i + theatreChaseQ, stripColor);
	}

	strip.show();

	++theatreChaseQ;
	if(theatreChaseQ > 3) {
		theatreChaseQ = 0;
		--chaseCycle;
	}
}

//
// Demo timer callback
//

void startDemo()
{
	Serial << _F("NeoPixel Demo type: ") << stripDemoType << endl;

	if(stripDemoType >= ARRAY_SIZE(demoData)) {
		// Demo loop restart
		stripDemoType = 0;
		timer.initializeMs<1000>(startDemo).start();
		return;
	}

	auto& data = demoData[stripDemoType];
	chaseCycle = data.chase;
	stripColor = data.color;
	timer.initializeMs(data.stepMilliseconds, chaseCycle ? theatreChase : colorWipe).start();
}

#ifndef DISABLE_WIFI
void gotIp(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial << "IP: " << ip << endl;
	// You can put here other job like web,tcp etc.
}

// Will be called when WiFi station loses connection
void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial.println(_F("I'm NOT CONNECTED!"));
}
#endif

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output to serial

	Serial.print(_F("NeoPixel demo .. start"));

#ifndef DISABLE_WIFI
	// Wifi could be used eg. for switching Neopixel from internet.
	// If not needed compile this sample with `make DISABLE_WIFI=1`

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIp);
#endif

	stripDemoType = 0;
	strip.begin();

	timer.initializeMs<1000>(startDemo).start();
}
