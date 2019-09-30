/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 * APA102 library by HappyCodingRobot@github.com
 ****/

#pragma once

#include <SPIBase.h>
#include <SPISettings.h>

struct col_t {
	uint8_t br;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

class APA102
{
public:
	APA102(uint16_t n);
	APA102(uint16_t n, SPIBase& spiRef);

	~APA102()
	{
		delete LEDbuffer;
	}

	void begin();
	void begin(SPISettings& mySettings);
	void end();

	/* send data buffer to LEDs, including start & stop sequences */
	void show();
	void show(uint16_t startPos);

	/* clear data buffer */
	void clear();

	/* set pixel color */
	void setPixel(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
	void setPixel(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t br);

	void setPixel(uint16_t n, const col_t& col);

	void setPixel(uint16_t n, const col_t* col)
	{
		if(col != nullptr) {
			setPixel(n, *col);
		}
	}

	void setAllPixel(uint8_t r, uint8_t g, uint8_t b);

	void setAllPixel(const col_t& col);

	void setAllPixel(const col_t* col)
	{
		if(col != nullptr) {
			setAllPixel(*col);
		}
	}

	/* set global LED brightness level */
	void setBrightness(uint8_t br);

	/* get global LED brightness level */
	uint8_t getBrightness() const
	{
		return brightness;
	}

	/* send start sequence */
	void sendStart();
	/* send stop sequence */
	void sendStop();
	/* direct write single LED data */
	void directWrite(uint8_t r, uint8_t g, uint8_t b, uint8_t br);

protected:
	uint16_t numLEDs = 0;
	uint8_t brightness = 0; // global brightness 0..31 -> 0..100%
	col_t* LEDbuffer = nullptr;

	SPISettings SPI_APA_Settings;
	SPIBase& pSPI;

private:
};
