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

// Prototypes
void updateLED();
col_t colorWheel(uint16_t step, uint16_t numStep);
void init();

void updateLED()
{
	static unsigned cnt = 0;

	if(cnt < NUM_LED) {
		cnt++;
	} else {
		cnt = 0;
	}

	Serial.printf("ping %u\n", cnt);
	LED.show(cnt); // show shifted LED buffer

	WDT.alive();
}

void init()
{
	WDT.enable(false);
	WifiAccessPoint.enable(false);
	WifiStation.enable(false);
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	/* configure SPI */
	LED.begin(); // default 4MHz clk, CS on PIN_2
	//LED.begin(SPI_1MHZ);
	//LED.begin(SPI_2MHZ);

	Serial.printf("start\n");
	LED.setBrightness(10); // brightness [0..31]
	LED.clear();

	LED.setAllPixel(255, 0, 0); // set all pixel to red
	LED.show();
	delay(500);

	LED.setAllPixel(0, 255, 0); // set all pixel to green
	LED.show();
	delay(500);

	LED.setPixel(10, pixel); // set single pixel
	LED.show();
	delay(500);

	for(int i = 0; i < NUM_LED; i++) { // some rainbow ..
		auto pixel = colorWheel(i, NUM_LED);
		LED.setPixel(i, pixel);
	}
	LED.show();

	procTimer.initializeMs<100>(updateLED).start();
}

/* color wheel function:
 * (simple) three 120° shifted colors -> color transitions r-g-b-r */
col_t colorWheel(uint16_t step, uint16_t numStep)
{
	col_t col = {0};
	uint8_t index = ((uint32_t)(step * 256) / numStep) & 255;
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
