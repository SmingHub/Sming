#include <SeDisplay.h>
#include <S1D13781/fonts.h>
#include <debug_progmem.h>

void SeDisplay::initialise()
{
	gfx.setLutDefault(Window::main);
	gfx.fillWindow(Window::main, bgColor);
	font.load(fontTable[font_sun8x16]);
}

void SeDisplay::drawString(uint16_t x, uint16_t y, const char* text)
{
	gfx.drawText(Window::main, font, text, x, y, 0, fgColor, bgColor, false);
}

void SeDisplay::drawChar(uint16_t x, uint16_t y, uint8_t c)
{
	char s[2] = {char(c), '\0'};
	gfx.drawText(Window::main, font, s, x, y, 0, fgColor, bgColor, false);
}

void SeDisplay::setBackColor(uint16_t col)
{
	if(bgColor.code != col) {
		bgColor = SeColor(col, format_RGB_565);
		debug_i("setBackColor(0x%04x)", col);
	}
}

void SeDisplay::setFrontColor(uint16_t col)
{
	if(fgColor.code != col) {
		fgColor = SeColor(col, format_RGB_565);
		debug_i("setFrontColor(0x%04x)", col);
	}
}

void SeDisplay::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t col)
{
	SeColor color(col, format_RGB_565);
	gfx.bltSolidFill(Window::main, SePos(x, y), SeSize(w, h), color);
}

void SeDisplay::scroll(uint16_t top, uint16_t bottom, int16_t diff)
{
	auto w = getWidth();
	auto h = 1 + bottom - top;
	if(diff < 0) {
		gfx.bltMove(Window::main, BltCmd::moveNegative, SePos(0, bottom + diff), SePos(0, bottom), SeSize(w, h + diff));
	} else if(diff > 0) {
		gfx.bltMove(Window::main, BltCmd::movePositive, SePos(0, top + diff), SePos(0, top), SeSize(w, h - diff));
	}
}

uint16_t SeDisplay::getWidth()
{
	return gfx.getWidth(Window::main);
}

uint16_t SeDisplay::getHeight()
{
	return gfx.getHeight(Window::main);
}

uint8_t SeDisplay::getCharWidth()
{
	return font.getCharWidth('X');
}

uint8_t SeDisplay::getCharHeight()
{
	return font.getHeight();
}
