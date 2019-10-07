/*
 * File: APA102 LED class demo for Sming framework
 * 
 * Original Author: https://github.com/HappyCodingRobot
 *
 * This library support the APA102 LED with integrated controller chip.
 * 
 *
 * hardware SPI: clk=GPIO14 , mosi=GPIO13
 * software SPI: user defined
 * 
 */
#include <SmingCore.h>

// SPI: if defined use software SPI, else hardware SPI
//#define _USE_SOFTSPI

#include <SPI.h>
#include <SPISoft.h>
#include <Libraries/APA102/apa102.h>

#define NUM_LED 60 // number of LEDs on strip

#define SPI_SCLK 14
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_CS 2

Timer procTimer;

// in this demo, the same ports for HW and SW SPI are used
#ifdef _USE_SOFTSPI
SPISoft sSPI(SPI_MISO, SPI_MOSI, SPI_SCLK, 200);
APA102 LED(NUM_LED, sSPI); // APA102 constructor for software SPI, call with number of LEDs
#else
APA102 LED(NUM_LED); // APA102 constructor, call with number of LEDs
					 //APA102 LED(NUM_LED, SPI);
#endif

static SPISettings SPI_1MHZ = SPISettings(1000000, MSBFIRST, SPI_MODE3);
static SPISettings SPI_2MHZ = SPISettings(2000000, MSBFIRST, SPI_MODE3);

/* color wheel function:
 * (simple) three 120° shifted colors -> color transitions r-g-b-r */
static col_t colorWheel(uint16_t step, uint16_t numStep)
{
	col_t col = {0};
	col.br = 10;
	uint8_t index = (step * 256) / numStep;
	uint8_t phase = 255 - index;
	if(phase < 85) { // 256/3 -> 2pi/3 -> 120°
		col.r = 255 - phase * 3;
		col.g = 0;
		col.b = phase * 3;
	} else if(phase < 170) {
		phase -= 85;
		col.r = 0;
		col.g = phase * 3;
		col.b = 255 - phase * 3;
	} else {
		phase -= 170;
		col.r = phase * 3;
		col.g = 255 - phase * 3;
		col.b = 0;
	}
	return col;
}

static void updateLED()
{
	static unsigned state = 0;
	static unsigned cnt = 0;

	switch(state++) {
	case 0:
		LED.setBrightness(10); // Default brightness [0..31]
		LED.clear();

		Serial.println(_F("Start: set all pixels to red"));
		LED.setAllPixel(255, 0, 0);
		LED.show();
		break;

	case 1:
		Serial.println(_F("Set all pixels to green"));
		LED.setAllPixel(0, 255, 0);
		LED.show();
		break;

	case 2:
		Serial.println(_F("Set pixel #10 to red"));
		LED.setPixel(10, 255, 0, 0);
		LED.show();
		break;

	case 3:
		Serial.println(_F("Start cycling through rainbow patterns"));
		for(unsigned i = 0; i < NUM_LED; i++) { // some rainbow ..
			auto pixel = colorWheel(i, NUM_LED);
			LED.setPixel(i, pixel);
		}
		LED.show();
		procTimer.setIntervalMs<100>();
		break;

	default:
		if(cnt < NUM_LED) {
			cnt++;
		} else {
			cnt = 0;
		}

		Serial.print("ping ");
		Serial.println(cnt);
		LED.show(cnt); // show shifted LED buffer
		break;
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	WifiAccessPoint.enable(false);
	WifiStation.enable(false);

	/* configure SPI */
	LED.begin(); // default 4MHz clk, CS on PIN_2
	//LED.begin(SPI_1MHZ);
	//LED.begin(SPI_2MHZ);

	procTimer.initializeMs<500>(updateLED).start();
}
