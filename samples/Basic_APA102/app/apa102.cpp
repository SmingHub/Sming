/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// APA102 LED class

#include <SmingCore/SmingCore.h>
#include <SPI.h>
#include "apa102.h"
//#include "hwSPI.h"


#define LED_PREAMBLE     (uint8_t)0xE0          // LED frame preamble
#define LED_PREAMBLELONG (uint32_t)0xE0000000   // LED frame preamble
#define brOfs 0
#define bOfs  1
#define gOfs  2
#define rOfs  3

//APA102 LED;

APA102::APA102(uint16_t n) : numLEDs(n), brightness(0), LEDbuffer(NULL) {
    if (LEDbuffer = (uint8_t *) malloc(numLEDs * 4)) {
        APA102::clear();
    }
}

APA102::~APA102() {
    if (LEDbuffer) free(LEDbuffer);
}

void APA102::begin(void) {
    //SPI.begin(10,4);       // 2MHz clk
    //SPI.begin(10, 8); // 1MHz clk
}

void APA102::end() {
    SPI.end();
}

void APA102::show(void) {
    uint32_t *buf = (uint32_t*) LEDbuffer;
    uint32_t elem;
    sendStart();
    for (uint16_t i = 0; i < numLEDs; i++) {
        elem = buf[i];
        SPI.transfer((uint8_t*)&elem, 4);
    }
    sendStop();
}

void APA102::show(int16_t SPos) {
    uint32_t *buf = (uint32_t*) LEDbuffer;
    uint32_t elem;
    int sp = numLEDs - (SPos % numLEDs);
    sendStart();
    for (int i = 0; i < numLEDs; i++) {
        elem = buf[(i + sp) % numLEDs];
        SPI.transfer((uint8_t*)&elem, 4);
    }
    sendStop();
}

void APA102::clear(void) {
    for (uint16_t i = 0; i < numLEDs; i++) {
        LEDbuffer[i * 4] = LED_PREAMBLE;
        LEDbuffer[i * 4 + bOfs] = 0;
        LEDbuffer[i * 4 + gOfs] = 0;
        LEDbuffer[i * 4 + rOfs] = 0;
    }
}

void APA102::setPixel(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (n < numLEDs) {
        LEDbuffer[n * 4] = LED_PREAMBLE | brightness;
        LEDbuffer[n * 4 + bOfs] = b;
        LEDbuffer[n * 4 + gOfs] = g;
        LEDbuffer[n * 4 + rOfs] = r;
    }
}

void APA102::setPixel(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t br) {
    if (n < numLEDs) {
        LEDbuffer[n * 4] = LED_PREAMBLE | (br < 32 ? br : 31);
        LEDbuffer[n * 4 + bOfs] = b;
        LEDbuffer[n * 4 + gOfs] = g;
        LEDbuffer[n * 4 + rOfs] = r;
    }
}

void APA102::setPixel(uint16_t n, col_t* p) {
    if (n < numLEDs) {
        LEDbuffer[n * 4] = LED_PREAMBLE | brightness;
        LEDbuffer[n * 4 + bOfs] = p->b;
        LEDbuffer[n * 4 + gOfs] = p->g;
        LEDbuffer[n * 4 + rOfs] = p->r;
    }
}

void APA102::setAllPixel(uint8_t r, uint8_t g, uint8_t b) {
    for (uint16_t i = 0; i < numLEDs; i++) {
        LEDbuffer[i * 4] = LED_PREAMBLE | brightness;
        LEDbuffer[i * 4 + bOfs] = b;
        LEDbuffer[i * 4 + gOfs] = g;
        LEDbuffer[i * 4 + rOfs] = r;
    }
}

void APA102::setAllPixel(col_t* p) {
    for (uint16_t i = 0; i < numLEDs; i++) {
        LEDbuffer[i * 4] = LED_PREAMBLE | brightness;
        LEDbuffer[i * 4 + bOfs] = p->b;
        LEDbuffer[i * 4 + gOfs] = p->g;
        LEDbuffer[i * 4 + rOfs] = p->r;
    }
}

void APA102::setBrightness(uint8_t br) {
    brightness = (br < 32 ? br : 31);
}

uint8_t APA102::getBrightness(void) {
    return brightness;
}

//void end(void); -> in destructor ??

/* direct write functions */

void APA102::sendStart(void) {
    uint8_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
    SPI.transfer(startFrame, sizeof (startFrame));
}

void APA102::sendStop(void) {
    uint8_t stopFrame[] = {0xff, 0xff, 0xff, 0xff};
    SPI.transfer(stopFrame, sizeof (stopFrame));
}

void APA102::directWrite(uint8_t r, uint8_t g, uint8_t b, uint8_t br) {
    uint8_t pix[4];
    pix[0] = 0xE0 | (br < 32 ? br : 31);
    pix[bOfs] = b;
    pix[gOfs] = g;
    pix[rOfs] = r;
    SPI.transfer(pix, sizeof (pix));
}


