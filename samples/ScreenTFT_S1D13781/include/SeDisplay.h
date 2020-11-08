#pragma once
#include <VT100/Display.h>
#include <S1D13781/Gfx.h>

class SeDisplay : public VT100::Display
{
public:
	using Window = S1D13781::Window;
	using BltCmd = S1D13781::BltCmd;

	SeDisplay(S1D13781::Gfx& gfx) : gfx(gfx)
	{
	}

	void initialise();

	/* Display */

	void drawString(uint16_t x, uint16_t y, const char* text) override;
	void drawChar(uint16_t x, uint16_t y, uint8_t c) override;
	void setBackColor(uint16_t col) override;
	void setFrontColor(uint16_t col) override;
	void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t col) override;
	void scroll(uint16_t top, uint16_t bottom, int16_t diff) override;
	uint16_t getWidth() override;
	uint16_t getHeight() override;
	uint8_t getCharWidth() override;
	uint8_t getCharHeight() override;

private:
	S1D13781::Gfx& gfx;
	SeFont font;
	SeColor fgColor;
	SeColor bgColor;
};
