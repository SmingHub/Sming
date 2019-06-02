/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 * APA102 library by HappyCodingRobot@github.com
 ****/
#include <SmingCore.h>
// APA102 LED class
#include <SPI.h>
#include <SPISoft.h>
#include "apa102.h"


#define LED_PREAMBLE     (uint8_t)0xE0          // LED frame preamble
#define LED_PREAMBLELONG (uint32_t)0xE0000000   // LED frame preamble
#define brOfs 0
#define bOfs  1
#define gOfs  2
#define rOfs  3



/* APA102 class for hardware & software SPI */

APA102::APA102(uint16_t n) : numLEDs(n), brightness(0), LEDbuffer(NULL), pSPI(SPI) {
    if (LEDbuffer = (uint8_t *) malloc(numLEDs * 4)) {
        clear();
    }
}

APA102::APA102(uint16_t n, SPIBase & spiRef) : numLEDs(n), brightness(0), LEDbuffer(NULL), pSPI(spiRef) {
    if (LEDbuffer = (uint8_t *) malloc(numLEDs * 4)) {
        clear();
    }
}

APA102::~APA102() {
    if (LEDbuffer) free(LEDbuffer);
}


void APA102::begin(void) {
    pSPI.begin();
}

void APA102::begin(SPISettings & mySettings) {
    pSPI.begin();
    SPI_APA_Settings = mySettings;
}

void APA102::end() {
    pSPI.end();
}



void APA102::show(void) {
    uint32_t *buf = (uint32_t*) LEDbuffer;
    uint32_t elem;
    pSPI.beginTransaction(SPI_APA_Settings);
    sendStart();
    for (uint16_t i = 0; i < numLEDs; i++) {
        elem = buf[i];
        pSPI.transfer((uint8_t*)&elem, 4);
    }
    sendStop();
    pSPI.endTransaction();
}

void APA102::show(int16_t SPos) {
    uint32_t *buf = (uint32_t*) LEDbuffer;
    uint32_t elem;
    pSPI.beginTransaction(SPI_APA_Settings);
    int sp = numLEDs - (SPos % numLEDs);
    sendStart();
    for (int i = 0; i < numLEDs; i++) {
        elem = buf[(i + sp) % numLEDs];
        pSPI.transfer((uint8_t*)&elem, 4);
    }
    sendStop();
    pSPI.endTransaction();
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

/* direct write functions */

inline void APA102::sendStart(void) {
    uint8_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
    pSPI.transfer(startFrame, sizeof (startFrame));
}

inline void APA102::sendStop(void) {
    uint8_t stopFrame[] = {0xff, 0xff, 0xff, 0xff};
    pSPI.transfer(stopFrame, sizeof (stopFrame));
}

void APA102::directWrite(uint8_t r, uint8_t g, uint8_t b, uint8_t br) {
    uint8_t pix[4];
    pSPI.beginTransaction(SPI_APA_Settings);
    pix[0] = 0xE0 | (br < 32 ? br : 31);
    pix[bOfs] = b;
    pix[gOfs] = g;
    pix[rOfs] = r;
    pSPI.transfer(pix, sizeof (pix));
}

