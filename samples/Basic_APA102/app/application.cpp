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
#include "user_config.h"
#include <SmingCore/SmingCore.h>

// SPI: if defined use software SPI, else hardware SPI
//#define _USE_SOFTSPI

#ifdef _USE_SOFTSPI             // set in "user_config.h"
#include <SPISoft.h>
#else
#include <SPI.h>
#endif

#include <Libraries/APA102/apa102.h>

#define NUM_LED 60              // number of LEDs on strip

Timer procTimer;

#ifdef _USE_SOFTSPI
SPISoft sSPI(12,13,14,2);       // mosi=GPIO13, clk=GPIO14
APA102Soft LED(NUM_LED, &sSPI); // APA102 constructor for software SPI, call with number of LEDs
#else
APA102 LED(NUM_LED);            // APA102 constructor, call with number of LEDs
#endif

int cnt = 0;
col_t pixel;

// Prototypes
void updateLED(void);
void colorWheel(uint16_t step, uint16_t numStep, col_t* c);
void init(void);



void updateLED() {
    if (cnt < NUM_LED) {
        cnt++;
    } else {
        cnt = 0;
    }
    
    Serial.printf("ping %u\n",cnt);
    LED.show(cnt);                          // show shifted LED buffer
    
    WDT.alive();
}


void init() {
    WDT.enable(false);
    WifiAccessPoint.enable(false);
    WifiStation.enable(false);
    Serial.begin(SERIAL_BAUD_RATE);       // 115200 by default
    /* configure SPI */
#ifdef _USE_SOFTSPI
    LED.begin();
#else
    LED.begin();              // default 1MHz clk
    //LED.begin(10,2);        // 4MHz clk @ f_cpu==80MHz
    //LED.begin(10,4);        // 2MHz clk @ f_cpu==80MHz
    //LED.begin(10,8);        // 1MHz clk @ f_cpu==80MHz
    //LED.begin(10,16);       // 512kHz clk @ f_cpu==80MHz
#endif
    Serial.printf("start\n");
    LED.setBrightness(10);          // brightness [0..31]
    LED.clear();
    
    pixel.r = 255; pixel.g = 0; pixel.b =0;
    LED.setAllPixel(&pixel);                // set all pixel to red
    LED.show();
    delay(500);
    
    LED.setAllPixel(0,255,0);               // set all pixel to green
    LED.show();
    delay(500);
    
    LED.setPixel(10,&pixel);                // set single pixel
    LED.show();
    delay(500);
    
    for (int i=0; i<NUM_LED; i++) {         // some rainbow ..
        colorWheel(i, NUM_LED, &pixel);
        LED.setPixel(i, &pixel);
    }
    LED.show();
    
    procTimer.initializeMs(100, updateLED).start();
    
}


/* color wheel function:
 * (simple) three 120° shifted colors -> color transitions r-g-b-r */
void colorWheel(uint16_t step, uint16_t numStep, col_t* c) {
    uint8_t index = ((uint32_t) (step * 256) / numStep) & 255;
    uint8_t phase = 255 - index;
    if (phase < 85) { // 256/3 -> 2pi/3 -> 120°
        c->r = 255 - phase * 3;
        c->g = 0;
        c->b = phase * 3;
    } else if (phase < 170) {
        phase -= 85;
        c->r = 0;
        c->g = phase * 3;
        c->b = 255 - phase * 3;
    } else {
        phase -= 170;
        c->r = phase * 3;
        c->g = 255 - phase * 3;
        c->b = 0;
    }
}

