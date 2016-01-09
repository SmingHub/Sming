#ifndef _BMPDRAWH_
#define _BMPDRAWH_

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the esp8266's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

void bmpDraw(Adafruit_ST7735 tft, String filename, uint8_t x, uint8_t y);

// These read 16- and 32-bit types from the spifs file.
// BMP data is stored little-endian, esp8266 is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(file_t f);

uint32_t read32(file_t f);

#endif
