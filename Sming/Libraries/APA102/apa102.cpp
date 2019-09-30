/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 * APA102 library by HappyCodingRobot@github.com
 ****/

#include "apa102.h"
#include <SPI.h>

static constexpr uint8_t LED_PREAMBLE = 0xE0; // LED frame preamble

/* APA102 class for hardware & software SPI */

APA102::APA102(uint16_t n) : APA102(n, SPI)
{
}

APA102::APA102(uint16_t n, SPIBase& spiRef)
	: numLEDs(n), LEDbuffer(new col_t[numLEDs]), SPI_APA_Settings(4000000, MSBFIRST, SPI_MODE3), pSPI(spiRef)
{
	if(LEDbuffer == nullptr) {
		numLEDs = 0;
	} else {
		clear();
	}
}

void APA102::show(void)
{
	pSPI.beginTransaction(SPI_APA_Settings);
	sendStart();
	for(unsigned i = 0; i < numLEDs; i++) {
		auto col = LEDbuffer[i];
		pSPI.transfer(reinterpret_cast<uint8_t*>(&col), sizeof(col));
	}
	sendStop();
	pSPI.endTransaction();
}

void APA102::show(uint16_t startPos)
{
	pSPI.beginTransaction(SPI_APA_Settings);
	unsigned sp = numLEDs - (startPos % numLEDs);
	sendStart();
	for(unsigned i = 0; i < numLEDs; i++) {
		auto col = LEDbuffer[(i + sp) % numLEDs];
		pSPI.transfer(reinterpret_cast<uint8_t*>(&col), sizeof(col));
	}
	sendStop();
	pSPI.endTransaction();
}

void APA102::clear(void)
{
	for(unsigned i = 0; i < numLEDs; i++) {
		LEDbuffer[i] = {LED_PREAMBLE, 0, 0, 0};
	}
}

void APA102::setPixel(uint16_t n, const col_t& col)
{
	if(n < numLEDs) {
		auto c = col;
		c.br = std::min(c.br, LED_BR_MAX) | LED_PREAMBLE;
		LEDbuffer[n] = c;
	}
}

void APA102::setAllPixel(const col_t& col)
{
	auto c = col;
	c.br = std::min(c.br, LED_BR_MAX) | LED_PREAMBLE;
	for(unsigned i = 0; i < numLEDs; i++) {
		LEDbuffer[i] = c;
	}
}

/* direct write functions */

inline void APA102::sendStart(void)
{
	uint8_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
	pSPI.transfer(startFrame, sizeof(startFrame));
}

inline void APA102::sendStop(void)
{
	uint8_t stopFrame[] = {0xff, 0xff, 0xff, 0xff};
	pSPI.transfer(stopFrame, sizeof(stopFrame));
}

void APA102::directWrite(uint8_t r, uint8_t g, uint8_t b, uint8_t br)
{
	br = std::min(br, LED_BR_MAX);
	col_t col = {uint8_t(LED_PREAMBLE | br), r, g, b};

	pSPI.beginTransaction(SPI_APA_Settings);
	pSPI.transfer(reinterpret_cast<uint8_t*>(&col), sizeof(col));
}
