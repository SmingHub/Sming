#pragma once

#include <S1D13781/Gfx.h>
#include <VT100/Terminal.h>
#include <Timer.h>
#include "SeDisplay.h"

DECLARE_FSTR(smingLogo)

class DisplayDemo : public VT100::Callbacks
{
public:
	DisplayDemo(S1D13781::Gfx& graphics) : gfx(graphics), display(graphics), terminal(display, *this)
	{
	}

	void start();
	void stop();

	/* VT100::Callbacks */

	void sendResponse(const char* str) override
	{
	}

	using Window = S1D13781::Window;

	void drawLogo();
	void randomFill(Window window);
	void randomPixels(Window window);
	void randomLines(Window window);
	void randomRectangles(Window window, bool filled);
	void demoLoop();
	void printDisplayConfig();
	void lcdMemCheck1();
	void test_vt100();

	S1D13781::Gfx& gfx;
	SeDisplay display;
	VT100::Terminal terminal;
	Timer demoTimer;
};
