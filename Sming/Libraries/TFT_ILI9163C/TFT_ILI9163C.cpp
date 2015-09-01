#include "TFT_ILI9163C.h"

#include <limits.h>

#include "../../SmingCore/Digital.h"
#include "../../SmingCore/SPI.h"
#include "pins_arduino.h"
#include "wiring_private.h"

//constructors
TFT_ILI9163C::TFT_ILI9163C(uint8_t cspin,uint8_t dcpin,uint8_t rstpin) : Adafruit_GFX(_TFTWIDTH,_TFTHEIGHT){
	_cs   = cspin;
	_rs   = dcpin;
	_rst  = rstpin;
	_sid  = _sclk = 0;
}


TFT_ILI9163C::TFT_ILI9163C(uint8_t CS, uint8_t DC) : Adafruit_GFX(_TFTWIDTH, _TFTHEIGHT) {
  _cs   = CS;
  _rs   = DC;
  _rst  = 0;
  _sid  = _sclk = 0;
}

//Arduino Uno, Leonardo, Mega, Teensy 2.0, etc
#ifdef __AVR__

inline void TFT_ILI9163C::spiwrite(uint8_t c){
    SPDR = c;
    while(!(SPSR & _BV(SPIF)));
}

void TFT_ILI9163C::writecommand(uint8_t c){
	*rsport &= ~rspinmask;//low
	*csport &= ~cspinmask;//low
	spiwrite(c);
	*csport |= cspinmask;//hi
}

void TFT_ILI9163C::writedata(uint8_t c){
	*rsport |=  rspinmask;
	*csport &= ~cspinmask;
	spiwrite(c);
	*csport |= cspinmask;
} 

void TFT_ILI9163C::writedata16(uint16_t d){
	*rsport |=  rspinmask;
	*csport &= ~cspinmask;
	spiwrite(d >> 8);
	spiwrite(d);
	*csport |= cspinmask;
} 

void TFT_ILI9163C::setBitrate(uint32_t n){
	if (n >= 8000000) {
		SPI.setClockDivider(SPI_CLOCK_DIV2);
	} else if (n >= 4000000) {
		SPI.setClockDivider(SPI_CLOCK_DIV4);
	} else if (n >= 2000000) {
		SPI.setClockDivider(SPI_CLOCK_DIV8);
	} else {
		SPI.setClockDivider(SPI_CLOCK_DIV16);
	}
}
#elif defined(__SAM3X8E__)
// Arduino Due

inline void TFT_ILI9163C::spiwrite(uint8_t c){
    SPI.transfer(c);
}

void TFT_ILI9163C::writecommand(uint8_t c){
	rsport->PIO_CODR |=  rspinmask;//LO
	csport->PIO_CODR  |=  cspinmask;//LO
	spiwrite(c);
	csport->PIO_SODR  |=  cspinmask;//HI
}

void TFT_ILI9163C::writedata(uint8_t c){
	rsport->PIO_SODR |=  rspinmask;//HI
	csport->PIO_CODR  |=  cspinmask;//LO
	spiwrite(c);
	csport->PIO_SODR  |=  cspinmask;//HI
} 

void TFT_ILI9163C::writedata16(uint16_t d){
	rsport->PIO_SODR |=  rspinmask;//HI
	csport->PIO_CODR  |=  cspinmask;//LO
	spiwrite(d >> 8);
	spiwrite(d);
	csport->PIO_SODR  |=  cspinmask;//HI
}


void TFT_ILI9163C::setBitrate(uint32_t n){
	uint32_t divider=1;
	while (divider < 255) {
		if (n >= 84000000 / divider) break;
		divider = divider - 1;
	}
	SPI.setClockDivider(divider);
}
#elif defined(__MK20DX128__) || defined(__MK20DX256__)
//Teensy 3.0 & 3.1  

void TFT_ILI9163C::writecommand(uint8_t c){
	
	#if defined(__DMASPI)
	SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0);
	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
	#else

	#endif
}

void TFT_ILI9163C::writedata(uint8_t c){
	#if defined(__DMASPI)
	SPI0.PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0);
	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
	#else

	#endif
}

void TFT_ILI9163C::writedata16(uint16_t d){
	#if defined(__DMASPI)
	SPI0.PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1);
	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
	#else

	#endif

}

static bool spi_pin_is_cs(uint8_t pin){
	if (pin == 2 || pin == 6 || pin == 9) return true;
	if (pin == 10 || pin == 15) return true;
	if (pin >= 20 && pin <= 23) return true;
	return false;
}

static uint8_t spi_configure_cs_pin(uint8_t pin){
	switch (pin) {
		case 10: CORE_PIN10_CONFIG = PORT_PCR_MUX(2); return 0x01; // PTC4
		case 2:  CORE_PIN2_CONFIG  = PORT_PCR_MUX(2); return 0x01; // PTD0
		case 9:  CORE_PIN9_CONFIG  = PORT_PCR_MUX(2); return 0x02; // PTC3
		case 6:  CORE_PIN6_CONFIG  = PORT_PCR_MUX(2); return 0x02; // PTD4
		case 20: CORE_PIN20_CONFIG = PORT_PCR_MUX(2); return 0x04; // PTD5
		case 23: CORE_PIN23_CONFIG = PORT_PCR_MUX(2); return 0x04; // PTC2
		case 21: CORE_PIN21_CONFIG = PORT_PCR_MUX(2); return 0x08; // PTD6
		case 22: CORE_PIN22_CONFIG = PORT_PCR_MUX(2); return 0x08; // PTC1
		case 15: CORE_PIN15_CONFIG = PORT_PCR_MUX(2); return 0x10; // PTC0
    }
    return 0;
}

void TFT_ILI9163C::setBitrate(uint32_t n){
	if (n >= 24000000) {
		ctar = CTAR_24MHz;
	} else if (n >= 16000000) {
		ctar = CTAR_16MHz;
	} else if (n >= 12000000) {
		ctar = CTAR_12MHz;
	} else if (n >= 8000000) {
		ctar = CTAR_8MHz;
	} else if (n >= 6000000) {
		ctar = CTAR_6MHz;
	} else {
		ctar = CTAR_4MHz;
	}
	SIM_SCGC6 |= SIM_SCGC6_SPI0;
	SPI0.MCR = SPI_MCR_MDIS | SPI_MCR_HALT;
	SPI0.CTAR0 = ctar | SPI_CTAR_FMSZ(7);
	SPI0.CTAR1 = ctar | SPI_CTAR_FMSZ(15);
	SPI0.MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
}
#elif defined(__ESP8266_EX__)
// ESP8266 SOC

inline void TFT_ILI9163C::spiwrite(uint8_t c){
    SPI.transfer(c);
}

void TFT_ILI9163C::writecommand(uint8_t c){
	*rsport &= ~rspinmask;//low
	*csport &= ~cspinmask;//low
	spiwrite(c);
	*csport |= cspinmask;//hi
}

void TFT_ILI9163C::writedata(uint8_t c){
	*rsport |=  rspinmask;
	*csport &= ~cspinmask;
	spiwrite(c);
	*csport |= cspinmask;
}

void TFT_ILI9163C::writedata16(uint16_t d){
	*rsport |=  rspinmask;
	*csport &= ~cspinmask;
	spiwrite(d >> 8);
	spiwrite(d);
	*csport |= cspinmask;
}

#endif


void TFT_ILI9163C::begin(void) {
#ifdef __AVR__
	pinMode(_rs, OUTPUT);
	pinMode(_cs, OUTPUT);
	csport    = portOutputRegister(digitalPinToPort(_cs));
	rsport    = portOutputRegister(digitalPinToPort(_rs));
	cspinmask = digitalPinToBitMask(_cs);
	rspinmask = digitalPinToBitMask(_rs);
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz (half speed)
    //Due defaults to 4mHz (clock divider setting of 21)
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
	// toggle RST low to reset; CS low so it'll listen to us
	*csport &= ~cspinmask;
#elif defined(__SAM3X8E__)
	pinMode(_rs, OUTPUT);
	pinMode(_cs, OUTPUT);
	csport    = digitalPinToPort(_cs);
	rsport    = digitalPinToPort(_rs);
	cspinmask = digitalPinToBitMask(_cs);
	rspinmask = digitalPinToBitMask(_rs);
    SPI.begin();
    SPI.setClockDivider(21); // 4 MHz
    //Due defaults to 4mHz (clock divider setting of 21), but we'll set it anyway 
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
	// toggle RST low to reset; CS low so it'll listen to us
	csport ->PIO_CODR  |=  cspinmask; // Set control bits to LOW (idle)
#elif defined(__MK20DX128__) || defined(__MK20DX256__)
	_sid = 11;
	_sclk = 13;
	if (spi_pin_is_cs(_cs) && spi_pin_is_cs(_rs)
	 && (_sid == 7 || _sid == 11)
	 && (_sclk == 13 || _sclk == 14)
	 && !(_cs ==  2 && _rs == 10) && !(_rs ==  2 && _cs == 10)
	 && !(_cs ==  6 && _rs ==  9) && !(_rs ==  6 && _cs ==  9)
	 && !(_cs == 20 && _rs == 23) && !(_rs == 20 && _cs == 23)
	 && !(_cs == 21 && _rs == 22) && !(_rs == 21 && _cs == 22)) {
		if (_sclk == 13) {
			CORE_PIN13_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE;
			SPCR.setSCK(13);
		} else {
			CORE_PIN14_CONFIG = PORT_PCR_MUX(2);
			SPCR.setSCK(14);
		}
		if (_sid == 11) {
			CORE_PIN11_CONFIG = PORT_PCR_MUX(2);
			SPCR.setMOSI(11);
		} else {
			CORE_PIN7_CONFIG = PORT_PCR_MUX(2);
			SPCR.setMOSI(7);
		}
		ctar = CTAR_12MHz;
		pcs_data = spi_configure_cs_pin(_cs);
		pcs_command = pcs_data | spi_configure_cs_pin(_rs);
		SIM_SCGC6 |= SIM_SCGC6_SPI0;
		SPI0.MCR = SPI_MCR_MDIS | SPI_MCR_HALT;
		SPI0.CTAR0 = ctar | SPI_CTAR_FMSZ(7);
		SPI0.CTAR1 = ctar | SPI_CTAR_FMSZ(15);
		SPI0.MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
	} else {
		//error
	}
#elif defined(__ESP8266_EX__)
	// ESP8266 SOC
	pinMode(_rs, OUTPUT);
	pinMode(_cs, OUTPUT);
	csport    = portOutputRegister(digitalPinToPort(_cs));
	rsport    = portOutputRegister(digitalPinToPort(_rs));
	cspinmask = digitalPinToBitMask(_cs);
	rspinmask = digitalPinToBitMask(_rs);
	SPI.begin();
	//SPI.setBitOrder(MSBFIRST);
    //SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz (half speed) // Not implemented yet :(
    //Due defaults to 4mHz (clock divider setting of 21)
    //SPI.setDataMode(SPI_MODE0);
	// toggle RST low to reset; CS low so it'll listen to us
	*csport &= ~cspinmask;
#endif

  if (_rst != 0) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(500);
    digitalWrite(_rst, LOW);
    delay(500);
    digitalWrite(_rst, HIGH);
    delay(500);
  }

/*
7) MY:  1(bottom to top), 0(top to bottom) 	Row Address Order
6) MX:  1(R to L),        0(L to R)        	Column Address Order
5) MV:  1(Exchanged),     0(normal)        	Row/Column exchange
4) ML:  1(bottom to top), 0(top to bottom) 	Vertical Refresh Order
3) RGB: 1(BGR), 		   0(RGB)           	Color Space
2) MH:  1(R to L),        0(L to R)        	Horizontal Refresh Order
1)
0)

     MY, MX, MV, ML,RGB, MH, D1, D0
	 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0	//normal
	 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0	//Y-Mirror
	 0 | 1 | 0 | 0 | 1 | 0 | 0 | 0	//X-Mirror
	 1 | 1 | 0 | 0 | 1 | 0 | 0 | 0	//X-Y-Mirror
	 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0	//X-Y Exchange
	 1 | 0 | 1 | 0 | 1 | 0 | 0 | 0	//X-Y Exchange, Y-Mirror
	 0 | 1 | 1 | 0 | 1 | 0 | 0 | 0	//XY exchange
	 1 | 1 | 1 | 0 | 1 | 0 | 0 | 0
*/
  _Mactrl_Data = 0b00000000;
  _colorspaceData = __COLORSPC;//start with default data;
  chipInit();
}



void TFT_ILI9163C::chipInit() {
	writecommand(CMD_SWRESET);//software reset
	delay(500);
	writecommand(CMD_SLPOUT);//exit sleep
	delay(5);
	writecommand(CMD_PIXFMT);//Set Color Format 16bit   
	writedata(0x05);
	delay(5);
	writecommand(CMD_GAMMASET);//default gamma curve 3
	writedata(0x04);//0x04
	delay(1);
	writecommand(CMD_GAMRSEL);//Enable Gamma adj    
	writedata(0x01); 
	delay(1);
	writecommand(CMD_NORML);
	
	writecommand(CMD_DFUNCTR);
	writedata(0b11111111);//
	writedata(0b00000110);//

	writecommand(CMD_PGAMMAC);//Positive Gamma Correction Setting
	#if defined(__GAMMASET1)
		writedata(0x36);//p1
		writedata(0x29);//p2
		writedata(0x12);//p3
		writedata(0x22);//p4
		writedata(0x1C);//p5
		writedata(0x15);//p6
		writedata(0x42);//p7
		writedata(0xB7);//p8
		writedata(0x2F);//p9
		writedata(0x13);//p10
		writedata(0x12);//p11
		writedata(0x0A);//p12
		writedata(0x11);//p13
		writedata(0x0B);//p14
		writedata(0x06);//p15
	#else
		writedata(0x3F);//p1
		writedata(0x25);//p2
		writedata(0x1C);//p3
		writedata(0x1E);//p4
		writedata(0x20);//p5
		writedata(0x12);//p6
		writedata(0x2A);//p7
		writedata(0x90);//p8
		writedata(0x24);//p9
		writedata(0x11);//p10
		writedata(0x00);//p11
		writedata(0x00);//p12
		writedata(0x00);//p13
		writedata(0x00);//p14
		writedata(0x00);//p15
	#endif

	writecommand(CMD_NGAMMAC);//Negative Gamma Correction Setting
	#if defined(__GAMMASET1)
		writedata(0x09);//p1
		writedata(0x16);//p2
		writedata(0x2D);//p3
		writedata(0x0D);//p4
		writedata(0x13);//p5
		writedata(0x15);//p6
		writedata(0x40);//p7
		writedata(0x48);//p8
		writedata(0x53);//p9
		writedata(0x0C);//p10
		writedata(0x1D);//p11
		writedata(0x25);//p12
		writedata(0x2E);//p13
		writedata(0x34);//p14
		writedata(0x39);//p15
	#else
		writedata(0x20);//p1
		writedata(0x20);//p2
		writedata(0x20);//p3
		writedata(0x20);//p4
		writedata(0x05);//p5
		writedata(0x15);//p6
		writedata(0x00);//p7
		writedata(0xA7);//p8
		writedata(0x3D);//p9
		writedata(0x18);//p10
		writedata(0x25);//p11
		writedata(0x2A);//p12
		writedata(0x2B);//p13
		writedata(0x2B);//p14
		writedata(0x3A);//p15
	#endif

	writecommand(CMD_FRMCTR1);//Frame Rate Control (In normal mode/Full colors)
	writedata(0x08);//0x0C//0x08
	writedata(0x02);//0x14//0x08
	delay(1);
	writecommand(CMD_DINVCTR);//display inversion 
	writedata(0x07);
    delay(1);
	writecommand(CMD_PWCTR1);//Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD   
	writedata(0x0A);//4.30 - 0x0A
	writedata(0x02);//0x05
	delay(1);
	writecommand(CMD_PWCTR2);//Set BT[2:0] for AVDD & VCL & VGH & VGL   
	writedata(0x02);
	delay(1);
	writecommand(CMD_VCOMCTR1);//Set VMH[6:0] & VML[6:0] for VOMH & VCOML   
	writedata(0x50);//0x50
	writedata(99);//0x5b
	delay(1);
	writecommand(CMD_VCOMOFFS);
	writedata(0);//0x40
	delay(1);
  
	writecommand(CMD_CLMADRS);//Set Column Address  
	writedata(0x00); 
	writedata(0X00); 
	writedata(0X00); 
	writedata(_GRAMWIDTH); 
  
	writecommand(CMD_PGEADRS);//Set Page Address  
	writedata(0x00); 
	writedata(0X00); 
	writedata(0X00); 
	writedata(_GRAMHEIGH); 

	colorSpace(_colorspaceData);
	setRotation(0);
	writecommand(CMD_DISPON);//display ON 
	delay(1);
	writecommand(CMD_RAMWR);//Memory Write

	delay(1);
	fillScreen(BLACK);
}

/*
Colorspace selection:
0: RGB
1: GBR
*/
void TFT_ILI9163C::colorSpace(uint8_t cspace) {
	if (cspace < 1){
		bitClear(_Mactrl_Data,3);
	} else {
		bitSet(_Mactrl_Data,3);
	}
}


void TFT_ILI9163C::clearScreen(uint16_t color) {
	homeAddress();
	for (int px=0;px < _GRAMSIZE; px++){
		writedata16(color);
	}
}

void TFT_ILI9163C::homeAddress() {
	setAddrWindow(0x00,0x00,_GRAMWIDTH-1,_GRAMHEIGH-1);
}



void TFT_ILI9163C::setCursor(int16_t x, int16_t y) {
	if (boundaryCheck(x,y)) return;
	setAddrWindow(0x00,0x00,x,y);
	cursor_x = x;
	cursor_y = y;
}



void TFT_ILI9163C::pushColor(uint16_t color) {
	 writedata16(color);
}

void TFT_ILI9163C::drawPixel(int16_t x, int16_t y, uint16_t color) {
	if (boundaryCheck(x,y)) return;
	if ((x < 0) || (y < 0)) return;
	setAddrWindow(x,y,x+1,y+1);
	writedata16(color);
}


void TFT_ILI9163C::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	// Rudimentary clipping
	if (boundaryCheck(x,y)) return;
	if (((y + h) - 1) >= _height) h = _height-y;
	setAddrWindow(x,y,x,(y+h)-1);
	while (h--) {
		writedata16(color);
	}
}

bool TFT_ILI9163C::boundaryCheck(int16_t x,int16_t y){
	if ((x >= _width) || (y >= _height)) return true;
	return false;
}

void TFT_ILI9163C::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	// Rudimentary clipping
	if (boundaryCheck(x,y)) return;
	if (((x+w) - 1) >= _width)  w = _width-x;
	setAddrWindow(x,y,(x+w)-1,y);
	while (w--) {
		writedata16(color);
	}
}

void TFT_ILI9163C::fillScreen(uint16_t color) {
	clearScreen(color);
}

// fill a rectangle
void TFT_ILI9163C::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	if (boundaryCheck(x,y)) return;
	if (((x + w) - 1) >= _width)  w = _width  - x;
	if (((y + h) - 1) >= _height) h = _height - y;
	setAddrWindow(x,y,(x+w)-1,(y+h)-1);
	
	for (y = h;y > 0;y--) {
		for (x = w;x > 0;x--) {
			writedata16(color);
		}
	}
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color

uint16_t TFT_ILI9163C::Color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


void TFT_ILI9163C::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	writecommand(CMD_CLMADRS); // Column
	if (rotation == 0){
		writedata16(x0);
		writedata16(x1);
	} else if (rotation == 1){
		writedata16(x0 + __OFFSET);
		writedata16(x1 + __OFFSET);
	} else if (rotation == 2){
		writedata16(x0);
		writedata16(x1);
	} else {
		writedata16(x0);
		writedata16(x1);
	}

	writecommand(CMD_PGEADRS); // Page
	if (rotation == 0){
		writedata16(y0 + __OFFSET);
		writedata16(y1 + __OFFSET);
	} else if (rotation == 1){
		writedata16(y0);
		writedata16(y1);
	} else if (rotation == 2){
		writedata16(y0);
		writedata16(y1);
	} else {
		writedata16(y0);
		writedata16(y1);
	}
	writecommand(CMD_RAMWR); //Into RAM
}


void TFT_ILI9163C::setRotation(uint8_t m) {
	rotation = m % 4; // can't be higher than 3
	switch (rotation) {
	case 0:
		_Mactrl_Data = 0b00001000;
		_width  = _TFTWIDTH;
		_height = _TFTHEIGHT;//-__OFFSET;
		break;
	case 1:
		_Mactrl_Data = 0b01101000;
		_width  = _TFTHEIGHT;//-__OFFSET;
		_height = _TFTWIDTH;
		break;
	case 2:
		_Mactrl_Data = 0b11001000;
		_width  = _TFTWIDTH;
		_height = _TFTHEIGHT;//-__OFFSET;
		break;
	case 3:
		_Mactrl_Data = 0b10101000;
		_width  = _TFTHEIGHT;
		_height = _TFTWIDTH;//-__OFFSET;
		break;
	}
	colorSpace(_colorspaceData);
	writecommand(CMD_MADCTL);
	writedata(_Mactrl_Data);
}


void TFT_ILI9163C::invertDisplay(boolean i) {
	writecommand(i ? CMD_DINVON : CMD_DINVOF);
}
