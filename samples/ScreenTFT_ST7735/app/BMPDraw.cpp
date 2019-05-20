/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
/****************************************************
 * ported for Sming by H.Boettcher.
 * this implementation uses spifs storage instead of SD cards
 * hbottc@gmail.com
 ***************************************************/

#include <SmingCore.h>
#include <Libraries/Adafruit_ST7735/Adafruit_ST7735.h>
#include "BPMDraw.h"

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the esp8266's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

void bmpDraw(Adafruit_ST7735 tft, String fileName, uint8_t x, uint8_t y)
{
	file_t handle;

	int bmpWidth, bmpHeight;			// W+H in pixels
	uint8_t bmpDepth;					// Bit depth (currently must be 24)
	uint32_t bmpImageoffset;			// Start of image data in file
	uint32_t rowSize;					// Not always = bmpWidth; may have padding
	uint8_t sdbuffer[3 * BUFFPIXEL];	// pixel buffer (R+G+B per pixel)
	uint8_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer
	bool goodBmp = false;				// Set to true on valid header parse
	int w, h, row, col;
	uint8_t r, g, b;
	uint32_t pos = 0, startTime = millis();

	if((x >= tft.width()) || (y >= tft.height()))
		return;

	Serial.println();
	Serial.print("Loading image '");
	Serial.print(fileName);
	Serial.println('\'');

	handle = fileOpen(fileName.c_str(), eFO_ReadOnly);
	if(handle == -1) {
		debugf("File wasn't found: %s", fileName.c_str());
		fileClose(handle);
		return;
	}

	// Parse BMP header
	if(read16(handle) == 0x4D42) {				   // BMP signature
		debugf("File size: %d\n", read32(handle)); // get File Size
		(void)read32(handle);					   // Read & ignore creator bytes
		bmpImageoffset = read32(handle);		   // Start of image data
		debugf("Image Offset: %d\n", bmpImageoffset);
		debugf("Header size: %d\n", read32(handle)); // Read DIB header
		bmpWidth = read32(handle);
		bmpHeight = read32(handle);
		if(read16(handle) == 1) {	  // # planes -- must be '1'
			bmpDepth = read16(handle); // bits per pixel
			debugf("Bit Depth: %d\n", bmpDepth);
			if((bmpDepth == 24) && (read32(handle) == 0)) { // 0 = uncompressed
				goodBmp = true;								// Supported BMP format -- proceed!

				debugf("Image size: %d x %d\n", bmpWidth, bmpHeight);

				// BMP rows are padded (if needed) to 4-byte boundary
				rowSize = (bmpWidth * 3 + 3) & ~3;

				// If bmpHeight is negative, image is in top-down order.
				// This is not canon but has been observed in the wild.
				bool flip = true; // BMP is stored bottom-to-top
				if(bmpHeight < 0) {
					bmpHeight = -bmpHeight;
					flip = false;
				}

				// Crop area to be loaded
				w = bmpWidth;
				h = bmpHeight;
				if((x + w - 1) >= tft.width())
					w = tft.width() - x;
				if((y + h - 1) >= tft.height())
					h = tft.height() - y;

				// Set TFT address window to clipped image bounds
				tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

				for(row = 0; row < h; row++) { // For each scanline...

					// Seek to start of scan line.  It might seem labor-
					// intensive to be doing this on every line, but this
					// method covers a lot of gritty details like cropping
					// and scanline padding.  Also, the seek only takes
					// place if the file position actually needs to change
					// (avoids a lot of cluster math in SD library).
					if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
						pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
					else // Bitmap is stored top-to-bottom
						pos = bmpImageoffset + row * rowSize;
					if(fileTell(handle) != pos) {
						fileSeek(handle, pos, eSO_FileStart);
						buffidx = sizeof(sdbuffer); // Force buffer reload
					}
					for(col = 0; col < w; col++) { // For each pixel...
						// Time to read more pixel data?
						if(buffidx >= sizeof(sdbuffer)) { // Indeed
							fileRead(handle, sdbuffer, sizeof(sdbuffer));
							buffidx = 0; // Set index to beginning
						}

						// Convert pixel from BMP to TFT format, push to display
						b = sdbuffer[buffidx++];
						g = sdbuffer[buffidx++];
						r = sdbuffer[buffidx++];
						tft.pushColor(tft.Color565(r, g, b));
					} // end pixel
				}	 // end scanline
				Serial.printf("Loaded in %d ms\n", millis() - startTime);
			} // end goodBmp
		}
	}

	fileClose(handle);
	if(!goodBmp)
		Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the spifs file.
// BMP data is stored little-endian, esp8266 is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(file_t f)
{
	char bytes[2];
	fileRead(f, bytes, 2);
	return (bytes[1] << 8) + bytes[0];
}

uint32_t read32(file_t f)
{
	char bytes[4];
	fileRead(f, bytes, 4);
	return (bytes[3] << 24) + (bytes[2] << 16) + (bytes[1] << 8) + bytes[0];
}
