#include "Sparkfun_L6470.h"

#include <stdlib.h>

#include "../../SmingCore/Wire.h"
#include "../../SmingCore/pins_arduino.h"
#include "c:/Espressif/ESP8266_SDK/include/eagle_soc.h"

// reduces how much is refreshed, which speeds it up!
// originally derived from Steve Evans/JCW's mod but cleaned up and
// optimized
//#define enablePartialUpdate

Sparkfun_L6470::Sparkfun_L6470(int8_t SCLK, int8_t DIN, int8_t DC, int8_t CS,
		int8_t RST)
{
	_din = DIN;
	_sclk = SCLK;
	_dc = DC;
	_rst = RST;
	_cs = CS;
}

Sparkfun_L6470::Sparkfun_L6470(int8_t SCLK, int8_t DIN, int8_t DC, int8_t RST)
{
	_din = DIN;
	_sclk = SCLK;
	_dc = DC;
	_rst = RST;
	_cs = -1;
}

Sparkfun_L6470::Sparkfun_L6470(int8_t DC, int8_t CS, int8_t RST)
{
	// -1 for din and sclk specify using hardware SPI
	_din = -1;
	_sclk = -1;
	_dc = DC;
	_rst = RST;
	_cs = CS;
}

void Sparkfun_L6470::begin()
{
	/*
	 if (isHardwareSPI()) {
	 // Setup hardware SPI.
	 SPI.begin();
	 SPI.setClockDivider(PCD8544_SPI_CLOCK_DIV);
	 SPI.setDataMode(SPI_MODE0);
	 SPI.setBitOrder(MSBFIRST);
	 }
	 else {
	 */
	// Setup software SPI.
	// Set software SPI specific pin outputs.
	pinMode(_din, OUTPUT);
	pinMode(_sclk, OUTPUT);

	// Set software SPI ports and masks.
	clkport = portOutputRegister(digitalPinToPort(_sclk));
	clkpinmask = digitalPinToBitMask(_sclk);
	mosiport = portOutputRegister(digitalPinToPort(_din));
	mosipinmask = digitalPinToBitMask(_din);
//  }

// Set common pin outputs.
	pinMode(_dc, OUTPUT);
	if (_rst > 0)
		pinMode(_rst, OUTPUT);
	if (_cs > 0)
		pinMode(_cs, OUTPUT);

	// toggle RST low to reset
	if (_rst > 0)
	{
		digitalWrite(_rst, LOW);
		delay(500);
		digitalWrite(_rst, HIGH);
	}

}

inline void Sparkfun_L6470::spiWrite(uint8_t d)
{
	if (isHardwareSPI())
	{
		// Hardware SPI write.
		SPI.transfer(d);
	}
	else
	{
		// Software SPI write with bit banging.
		for (uint8_t bit = 0x80; bit; bit >>= 1)
		{
			*clkport &= ~clkpinmask;
			if (d & bit)
				*mosiport |= mosipinmask;
			else
				*mosiport &= ~mosipinmask;
			*clkport |= clkpinmask;
		}
	}
}

bool Sparkfun_L6470::isHardwareSPI()
{
	return (_din == -1 && _sclk == -1);
}

void Sparkfun_L6470::command(uint8_t c)
{
	digitalWrite(_dc, LOW);
	if (_cs > 0)
		digitalWrite(_cs, LOW);
	shiftOut(_din, _sclk, MSBFIRST, c);
	if (_cs > 0)
		digitalWrite(_cs, HIGH);
}

void Sparkfun_L6470::data(uint8_t c)
{
	digitalWrite(_dc, HIGH);
	if (_cs > 0)
		digitalWrite(_cs, LOW);
	shiftOut(_din, _sclk, MSBFIRST, c);
	if (_cs > 0)
		digitalWrite(_cs, HIGH);
}
