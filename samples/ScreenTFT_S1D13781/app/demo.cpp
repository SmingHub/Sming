/*------------------------------------------------------------------------------
 * s1d13781_GraphicsLibraryDemo.ino
 * Sketch example program to demonstrate the S1D13781 S5U13781R01C100 Library
 * Functions.
 *
 * Copyright(C) SEIKO EPSON CORPORATION 2015. All rights reserved.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 *------------------------------------------------------------------------------*/

#undef SMING_RELEASE

#include <demo.h>
#include <S1D13781/registers.h>
#include <S1D13781/fonts.h>
#include <Platform/Timers.h>
#include <SimpleTimer.h>
#include <VT100/Terminal.h>
#include <SeDisplay.h>
#include <Data/CStringArray.h>
#include "MemCheckState.h"

//#define ENABLE_PIP

// Raw image data
IMPORT_FSTR(epsonImage, PROJECT_DIR "/files/epson_image.bin")
IMPORT_FSTR(smingLogo, PROJECT_DIR "/files/sming_logo.raw")

extern HSPI::Controller spi;

namespace
{
using namespace S1D13781;

//sample text
DEFINE_FSTR_LOCAL(flash_titletext, "S1D13781");
IMPORT_FSTR_LOCAL(flash_s1d13781text, PROJECT_DIR "/files/text1.txt");

DEFINE_FSTR_LOCAL(flash_introtext, "S1D13781 Shield Graphics Library Demo");
IMPORT_FSTR_LOCAL(flash_gltext, PROJECT_DIR "/files/text2.txt");

bool lcdCheck(uint16_t result, const char* func)
{
	if(result == 0) {
		return true;
	} else {
		debug_e("%s() error %u", func, result);
		return false;
	}
}

uint32_t random(uint32_t min, uint32_t max)
{
	return min + (os_random() % (max - min));
}

} // namespace

/*
 * fill window with random color
 */
void DisplayDemo::randomFill(Window window)
{
	uint32_t color = random(aclBlack, aclWhite);
	auto result = gfx.fillWindow(window, color);
	lcdCheck(result, _F("fillWindow"));
}

/*
 * draw some random pixels of random colors
 */
void DisplayDemo::randomPixels(Window window)
{
	//fill main window back to white
	auto result = gfx.fillWindow(window, 0x00FFFFFF);
	lcdCheck(result, _F("fillWindow"));

	auto width = gfx.getWidth(window);
	auto height = gfx.getHeight(window);

	for(unsigned i = 0; i < 10000; i++) {
		uint32_t color = random(aclBlack, aclWhite);
		auto x = random(0, width - 1);
		auto y = random(0, height - 1);
		result = gfx.drawPixel(window, x, y, color, i == 0);
		if(!lcdCheck(result, _F("drawPixel"))) {
			break;
		}
		//			delay(10);
	}
}

/*
 * draw some random lines of random colors
 *
 */
void DisplayDemo::randomLines(Window window)
{
	//fill main window back to white
	auto result = gfx.fillWindow(window, aclWhite);
	lcdCheck(result, _F("fillWindow"));

	auto width = gfx.getWidth(window);
	auto height = gfx.getHeight(window);

	for(unsigned i = 0; i < 250; i++) {
		auto x = random(0, width - 1);
		auto y = random(0, height - 1);
		auto w = random(0, width);
		auto h = random(0, height);
		SeColor color(random(aclBlack, aclWhite));
		result = gfx.drawLine(window, x, y, w, h, color);
		if(!lcdCheck(result, _F("drawLine"))) {
			break;
		}
		//		delay(10);
	}
}

/*
 * draw some random rectangles (boxes)
 */
void DisplayDemo::randomRectangles(Window window, bool filled)
{
	//clear main window
	auto result = gfx.clearWindow(window);
	lcdCheck(result, _F("clearWindow"));

	auto width = gfx.getWidth(window);
	auto height = gfx.getHeight(window);

	for(unsigned i = 0; i < 100; i++) { //do outline boxes
		auto x = random(0, width - 10);
		auto y = random(0, height - 10);
		auto w = random(5, width - x);
		auto h = random(5, height - y);
		uint32_t color = random(aclBlack, aclWhite);
		if(filled) {
			result = gfx.drawFilledRect(window, x, y, w, h, color);
			//			result = gfx.drawFilledRectSlow(window, x, y, w, h, color);
		} else {
			result = gfx.drawRect(window, x, y, w, h, color);
		}
		if(!lcdCheck(result, _F("drawRect"))) {
			break;
		}
		//		delay(10);
	}
}

#define STATE                                                                                                          \
	++stateNumber;                                                                                                     \
	if(state == stateNumber)

/*------------------------------------------------------------------------------
 * demoLoop()
 * 
 *------------------------------------------------------------------------------*/
void DisplayDemo::demoLoop()
{
	static uint8_t state;
	uint8_t stateNumber = -1;

	static unsigned stateCounter = 0; // Some states have internal count

	unsigned nextState = state + 1; // By default, move to next state...
	unsigned stateDelay = 2000;		// ...after this time

	//
	uint16_t result;
	SeFont font;

	// Time each loop iteration
	ElapseTimer timer;

	STATE
	{
		//turn off pip until we need it
		gfx.pipSetDisplayMode(PipEffect::disabld);
		gfx.setColorDepth(Window::main, format_RGB_332LUT);
		gfx.setLutDefault(Window::main);

		// Display some useful information for debugging
		auto mainSize = gfx.getWindowSize(Window::main);
		debug_i("Main Window: %u x %u", mainSize.width, mainSize.height);

		auto pipPos = gfx.pipGetPosition();
		auto pipSize = gfx.getWindowSize(Window::pip);
		debug_i("PIP Window:  (%u, %u), %u x %u)", pipPos.x, pipPos.y, pipSize.width, pipSize.height);

		// fill main window with black
		result = gfx.fillWindow(Window::main, aclBlack);
		lcdCheck(result, _F("fillWindow"));

		// Chequerboard pattern to test timing and alignment
		SeColor color(0, format_RGB_332LUT);
		SeRect rc(0, 0, 35, 35);
		for(rc.y = 0; rc.y2() <= mainSize.height; rc.y += rc.height) {
			for(rc.x = 0; rc.x2() <= mainSize.width; rc.x += rc.width) {
				gfx.drawFilledRect(Window::main, rc, color);
				//				gfx.drawFilledRectSlow(Window::main, rc, color);
				++color.code;
			}
		}
	}

	STATE
	{
		//fill main window with white
		result = gfx.fillWindow(Window::main, aclWhite);
		lcdCheck(result, _F("fillWindow"));

		// Draw a colour gradient scale
		SeRect rc(0, 0, 30, 30);

		auto mainSize = gfx.getWindowSize(Window::main);

		for(uint8_t r = 0; r <= 7; ++r) {
			for(uint8_t g = 0; g <= 7; ++g) {
				for(uint8_t b = 0; b <= 3; ++b) {
					if(rc.y2() > mainSize.height) {
						break;
					}
					SeColor color((r << 5) | (g << 2) | b, format_RGB_332LUT);
					gfx.drawFilledRect(Window::main, rc, color);
					rc.x += rc.width;
					if(rc.x2() > mainSize.width) {
						rc.x = 0;
						rc.y += rc.height;
					}
				}
			}
		}
	}

	STATE
	{
		gfx.fillWindow(Window::main, aclWhite);

		auto mainSize = gfx.getWindowSize(Window::main);
		// Draw the epson Logo, centred
		unsigned imageWidth = 96;
		unsigned imageHeight = 34;
		unsigned x = (mainSize.width - imageWidth) / 2;
		unsigned y = 10;
		debug_i("drawImage(%u, %u, %u, %u)", x, y, imageWidth, imageHeight);
		gfx.drawImage(epsonImage, Window::main, x, y, imageWidth, imageHeight);
		y += imageHeight + 10;

		//now draw some text about the library using drawText()
		//1.create the font
		//2.draw the text
		//3.free the font (if not using further)
		font.load(fontTable[font_sun12x22]);

		//report the font being used
		debug_i("Font Name: %s", font.getName());

		//draw the title
		LOAD_FSTR(titletext, flash_titletext);
		debug_i("Drawing Title text: %s", titletext);
		gfx.drawText(Window::main, font, titletext, (mainSize.width - font.getTextWidth(titletext)) / 2, y, 0, aclBlack,
					 aclWhite, false);
		y += font.getHeight() + 5;

		//create smaller font
		font.load(fontTable[font_pearl_8x8]); // ascii9x13p);

		//now draw some text using drawMultiLineText()
		LOAD_FSTR(s1d13781text, flash_s1d13781text);
		debug_i("Drawing S1D13781 text: %s", s1d13781text);
		gfx.drawMultiLineText(Window::main, font, s1d13781text, 10, y, mainSize.width - 20, aclBlack, aclWhite, true);

		stateDelay *= 3;
	}

	STATE
	{
		randomFill(Window::main);
	}

	STATE
	{
		auto mainSize = gfx.getWindowSize(Window::main);
		//clear main window
		result = gfx.clearWindow(Window::main);
		lcdCheck(result, _F("clearWindow"));
		//delay(2000);

		//draw the introtext, this time in white
		LOAD_FSTR(introtext, flash_introtext);
		debug_i("Drawing Intro Text: %s", introtext);
		font.load(fontTable[font_10x18]);
		gfx.drawText(Window::main, font, introtext, (mainSize.width - font.getTextWidth(introtext)) / 2, 50, 0,
					 aclWhite, aclBlack, false);

		//create proportional font
		font.load(fontTable[font_acorn_8x8]);

		//now draw some more text using drawMultiLineText()
		LOAD_FSTR(gltext, flash_gltext);
		debug_i("Drawing S1D13781 text: %u", gltext);
		gfx.drawMultiLineText(Window::main, font, gltext, 10, 80, mainSize.width - 20, aclGreen, aclBlack, true);
	}

	STATE
	{
		auto mainSize = gfx.getWindowSize(Window::main);

		if(stateCounter == 0) {
			result = gfx.clearWindow(Window::main);
			lcdCheck(result, _F("clearWindow"));
		}

		// Fill screen with font tables

		const uint32_t colors[] = {aclWhite,	aclYellow,   aclCyan,		aclGreen,	 aclMagenta,
								   aclRed,		aclBlue,	 aclAquamarine, aclTurquoise, aclBisque,
								   aclCornsilk, aclDeepPink, aclLightSalmon};
		SePos pos;
		bool done = false;
		for(;;) {
			// Create font and draw chart
			font.load(fontTable[stateCounter]);

			char asciiChart[257];
			unsigned charCount = 0;
			for(unsigned c = 1; c < 256; ++c) {
				if(c == '\r' || c == '\n' || c == '\t' || c == ' ') {
					continue;
				}
				if(font.isDefined(c)) {
					asciiChart[charCount++] = c;
				}
			}
			asciiChart[charCount] = '\0';
			debug_i("Font '%s' has %u chars", font.getName(), charCount);

			auto color = colors[stateCounter % ARRAY_SIZE(colors)];
			gfx.scrollUp(Window::main, font.getHeight() * 3 / 2, aclBlack);
			pos.y = mainSize.height - font.getHeight();
			gfx.drawText(Window::main, font, font.getName(), pos.x, pos.y, 0, color, aclBlack, false);
			//			pos.y += font.getHeight() * 3 / 2;
			gfx.scrollUp(Window::main, font.getHeight() / 2, aclBlack);

			for(unsigned i = 0; i < charCount;) {
				gfx.scrollUp(Window::main, font.getHeight(), aclBlack);
				auto n = gfx.drawText(Window::main, font, &asciiChart[i], pos.x, pos.y, 0, scaleColor(color, 70),
									  aclBlack, false);
				i += n;
			}

			++stateCounter;
			if(stateCounter >= fontTable.length()) {
				done = true;
				break;
			}

			stateDelay = 1000;
			break;
		}

		if(done) {
			stateDelay *= 3;
		} else {
			nextState = state;
		}
	}

	STATE
	{
		randomPixels(Window::main);
	}

	STATE
	{
		randomLines(Window::main);
	}

	STATE
	{
		randomRectangles(Window::main, false);
	}

	STATE
	{
		randomRectangles(Window::main, true);
	}

	STATE
	{
		//draw Horizontal RGB color bars to the main window
		result = gfx.drawPattern(Window::main, patternRgbHorizBars, 50);
		lcdCheck(result, _F("drawPattern"));
	}

	STATE
	{
		//since we know the color at each location, test the get Pixel routine
		auto color = gfx.getPixel(Window::main, 10, 70);
		debug_i("Reading Pixel Value in Red third: 0x%08X", color); // should return RED 0x00FF0000
		color = gfx.getPixel(Window::main, 10, 200);
		debug_i("Reading Pixel Value in Green third: 0x%08X", color); // should return GREEN 0x0000FF00
		color = gfx.getPixel(Window::main, 10, 420);
		debug_i("Reading Pixel Value in Blue third: 0x%08X", color); // should return BLUE 0x000000FF
		color = gfx.getPixel(Window::invalid, 63, 300);
		debug_i("Reading Pixel from Invalid window: 0x%08X", color); // should return 0xFF000000
		color = gfx.getPixel(Window::main, 87, 500);
		debug_i("Reading Pixel Offscreen: 0x%08X", color); // should return 0xFE000000

		//draw Horizontal RGB gradients to the main window
		result = gfx.drawPattern(Window::main, patternRgbHorizGradient, 100);
		lcdCheck(result, _F("drawPattern"));
	}

	STATE
	{
		result = gfx.drawPattern(Window::main, patternVertBars, 100);
		lcdCheck(result, _F("drawPattern"));
	}

	STATE
	{
#ifdef ENABLE_PIP
		//test the pip window
		//create 32x16 PIP window (only memory we have left @24bpp)
		gfx.pipSetColorDepth(format_RGB_888);
		gfx.pipSetStartAddress(0x5FA00); // after main window (480*272*3 = 0x5FA00)
		gfx.pipSetSize(32, 16);
#endif

		//turn on pip
		gfx.pipSetDisplayMode(PipEffect::normal);

		//fill pip window with black
		result = gfx.fillWindow(Window::pip, aclBlack);
		lcdCheck(result, _F("fillWindow"));
	}

	STATE
	{
		//fill pip window with white
		result = gfx.fillWindow(Window::pip, aclWhite);
		lcdCheck(result, _F("fillWindow"));
	}

	STATE
	{
		randomFill(Window::pip);
	}

	STATE
	{
		//clear pip window to black
		result = gfx.clearWindow(Window::pip);
		lcdCheck(result, _F("ClearWindow"));
	}

	STATE
	{
		randomPixels(Window::pip);
	}

	STATE
	{
		randomLines(Window::pip);
	}

	STATE
	{
		randomRectangles(Window::pip, false);
	}

	STATE
	{
		randomRectangles(Window::pip, true);
	}

	STATE
	{
		//draw Horizontal RGB Bars to the pip window
		result = gfx.drawPattern(Window::pip, patternRgbHorizBars, 100);
		lcdCheck(result, _F("drawPattern"));
	}

	STATE
	{
		//since we know the colors in the pip window, test getPixel routine
		auto color = gfx.getPixel(Window::pip, 5, 2);
		debug_i("Reading Pixel Value in Red third: 0x%08X", color); // should get 0x00FF0000
		color = gfx.getPixel(Window::invalid, 63, 150);
		debug_i("Reading Pixel from Invalid Window: 0x%08X", color); //should get 0xFF000000
		color = gfx.getPixel(Window::pip, 87, 350);
		debug_i("Reading Pixel Offscreen: 0x%08X", color); //should get 0xFE000000

		//draw Horizontal RGB gradients to the pip window
		result = gfx.drawPattern(Window::pip, patternRgbHorizGradient, 100);
		lcdCheck(result, _F("drawPattern"));
	}

	STATE
	{
		//draw Vertical color bars on the display
		result = gfx.drawPattern(Window::pip, patternVertBars, 100);
		lcdCheck(result, _F("drawPattern"));
	}

	STATE
	{
		//do some pip window things
		auto displaySize = gfx.getDisplaySize();
		debug_i("displaySize = (%u, %u)", displaySize.width, displaySize.height);
		auto pipSize = gfx.getWindowSize(Window::pip);
		debug_i("pipSize = (%u, %u)", pipSize.width, pipSize.height);
		gfx.pipSetPosition((displaySize.width - pipSize.width) / 2, (displaySize.height - pipSize.height) / 2);

		//set blink rate
		gfx.pipSetFadeRate(32);
		gfx.pipSetDisplayMode(PipEffect::normal);
		stateDelay /= 4;
	}

	STATE
	{
		gfx.pipSetDisplayMode(PipEffect::blink1);
	}

	STATE
	{
		gfx.pipSetDisplayMode(PipEffect::blink2);
	}

	STATE
	{
		gfx.pipSetDisplayMode(PipEffect::normal);
		gfx.pipSetDisplayMode(PipEffect::normal);
		//set fade rate
		constexpr uint8_t fadeRate = 2;
		gfx.pipSetFadeRate(fadeRate); // adjust every 2 frames
		gfx.pipSetDisplayMode(PipEffect::continuous);
		//		auto steps = gfx.pipGetAlphaBlendStep();
		auto effectFrames = 2 * 64 / fadeRate; // Number of frames per fade cycle
		auto frames = gfx.getFrameCount(10000);
		// Round up frames to whole number of effect frames
		frames += effectFrames - (frames % effectFrames);
		//		stateDelay = gfx.getDisplayTime(frames);

		debug_i("Displaying %u frames, effects = %u frames, time = %u ms", frames, effectFrames, stateDelay);
	}

	STATE
	{
		if(stateCounter++ < 500) {
			const unsigned step = 1;
			auto displaySize = gfx.getDisplaySize();
			SeRect pip(gfx.pipGetPosition(), gfx.getWindowSize(Window::pip));

			//if new X position is "onscreen" then step X, otherwise reset to left
			pip.x += step;
			if(unsigned(pip.x2()) > displaySize.width) {
				pip.x = 0;
			}

			//if new Y position is "onscreen" then step Y, otherwise reset to top
			pip.y += step;
			if(unsigned(pip.y2()) > displaySize.height) {
				pip.y = 0;
			}
			gfx.pipSetPosition(pip.x, pip.y);
			stateDelay = 10;
			nextState = state;
		}
	}

	STATE
	{
		// Go back to start
		gfx.pipSetDisplayMode(PipEffect::disabld);
		nextState = 0;
	}

	auto& stats = gfx.controller.stats;
	debug_i("Demo loop # %u: elapsed = %s, waitCycles = %u, trans = %u, free heap = %u", state,
			timer.elapsedTime().toString().c_str(), stats.waitCycles, stats.transCount, system_get_free_heap_size());
	stats.clear();

	// Schedule next state
	if(nextState != state) {
		state = nextState;
		stateCounter = 0;
	}

	if(stateDelay == 0) {
		System.queueCallback([this]() { demoLoop(); });
	} else {
		demoTimer.setIntervalMs(stateDelay);
		demoTimer.startOnce();
	}
}

void DisplayDemo::printDisplayConfig()
{
	debug_i("LCD Configuration");

#define param(tag, fmt, ...) debug_i("%15s: " fmt, _F(tag), __VA_ARGS__)

	param("REV CODE", "%u", gfx.regRead(REG00_REV_CODE) << 8);

	auto data = gfx.regRead(REG02_PROD_CODE);
	param("PROD CODE", "0x%04X", data);

	uint16_t val = gfx.regRead(REG12_PLL_1);
	param("PLL N-Counter", "%u", val >> 10);
	param("PLL M-Divide", "%u:1", (val & 0x03FF) + 1);

	val = gfx.regRead(REG14_PLL_2);
	param("PLL L-Counter", "%u:1", (val & 0x03FF) + 1);

	val = gfx.regRead(REG16_INTCLK);
	param("PCLK Divide", "%u:1", (val & 0x000F) + 1);

	val = gfx.regRead(REG20_PANEL_SET);
	CStringArray deStrings(F("Active low\0Active high\0Fixed to low\0Fixed to high"));
	param("DE Polarity", "%s", deStrings.getValue((val >> 6) & 0x03));
	CStringArray risingFalling(F("Rising\0Falling"));
	param("PCLK Polarity", "%s edge", risingFalling.getValue((val >> 5) & 0x01));
	CStringArray panelTypes(F("4-bit mono\0"
							  "\x00"
							  "8-bit mono\0"
							  "\0"
							  "\0"
							  "\0"
							  "\0"
							  "\0"
							  "\0"
							  "\0"
							  "Single Color 8-bit Format 2\x00"
							  "16-bit TFT\0"
							  "\0"
							  "18-bit TFT\0"
							  "\0"
							  "24-bit TFT"));
	param("Panel type", "%u, %s", val & 0x0F, panelTypes.getValue(val & 0x0F));

	val = gfx.regRead(REG24_HDISP);
	param("HDISP", "%u", (val & 0x7F) * 8);

	val = gfx.regRead(REG26_HNDP);
	param("HNDP", "%u", val & 0x7F);

	val = gfx.regRead(REG28_VDISP);
	param("VDISP", "%u", val & 0x03FF);

	val = gfx.regRead(REG2A_VNDP);
	param("VNDP", "%u", val & 0xFF);

	val = gfx.regRead(REG2C_HSW);
	CStringArray highLow("high\0low");
	param("HSW", "%u, active %s", val & 0x7F, highLow.getValue((val >> 7) & 0x01));

	val = gfx.regRead(REG2E_HPS);
	param("HPS", "%u", val & 0x7F);

	val = gfx.regRead(REG30_VSW);
	param("VSW", "%u, active %s", val & 0x3F, highLow.getValue((val >> 7) & 0x01));

	val = gfx.regRead(REG32_VPS);
	param("VPS", "%u", val & 0xFF);

	auto& timing = gfx.getTiming();

	debug_i("N-Counter = %u", timing.nCounter);
	debug_i("M-Divider = %u", timing.mDivider);
	debug_i("L-Counter = %u", timing.lCounter);
	debug_i("PFDCLK = %u", timing.pfdclk);
	debug_i("MCLK = %u", timing.mclk);
	debug_i("PCLK = %u", timing.pclk);
	debug_i("Frame interval = %u (%0.2f fps)", timing.frameInterval, 1e6 / timing.frameInterval);

	debug_i("Bytes per pixel = %u, stride = %u", gfx.getBytesPerPixel(Window::main), gfx.getStride(Window::main));

#undef param
}

/*
 * Initial version of this used a uint8_t[], filling it with random(0, 255).
 * Filling the buffer took about 3ms.
 * This version, with uint32_t[] and os_random(), takes 160us.
 *
 * Standard blocking write / read takes 454ms for the check.
 * Double buffering:
 * 		Total time 403ms
 * 		Burst duration (csL - csH): 20.8us
 * 			1 cmd + 3 addr + 64 data bytes @ 26.7MHz clock
 * 		Gap between bursts (csH - csL): 8.9 - 25us (11.30 av.)
 *			ISR latency: 2us
 *			Transaction setup time: 6.4us
 * 		Gap between read / write transactions (csH - csL): 19us
 *
 * 		Other gap: 62us
 * 			ISR latency: 2us
 * 			ISR code: 2.6us
 * 			Idle (main code doing stuff): 50us
 * 			Transaction setup: 7.1us
 * 		(50us from ISR end to next one)
 *			= (buffer fill time) - (buffer read time) = (160 -
 *
 *
 *	Update:
 *		Main VRAM is actually 384K = 393216 bytes - so we do have 9216 spare bytes!
 *		Entire memory space including LUT1/2 checked, though they always return LSB = 0.
 *		Total time 383ms with 512 byte buffers
 *			Total bytes transferred (written + read) without overhead = 0x60800 x 2 = 0xC1000
 *			Average bitrate = 0xC1000 / 0.383 = 2.06 MB/s = 16.5Mbits/s, 62% bus utilisation
 *			Wait cycles = 991000, that's just checking RAM flag in a loop; that can be eliminated
 *			using completion callbacks for both read and write operations. (We'd use the system task
 *			queue to manage all that.)
 *		With synchronous writes: 398ms, 1096500 wait cycles
 *		With synchronous reads: 467ms, 1547100
 *		With synchronous reads and writes: 480ms, 1642816
 *
 *	The larger aim here is looking towards a pipeline approach. Much like modern 3D graphics cards,
 *	we buffer a load of high-level instructions then get the GPU to execute them asynchronously.
 *	The EVE GPU does exactly this, we give it instructions in terms of high-level primitives and
 *	it executes them. However, a brief look at the provided API shows that there's no local
 *	buffering for the commands; the assumption will be, presumably, that a quad SPI link at 30MHz
 *	is fast enough that intermediate buffering is un-necessary. One of the standout features
 *	of the EVE is it's 1MByte RAM; we could use that for I2S audio buffering, for example, or anything
 *	else. There are probably other uses for the GPU.
 *
 *	Given all that, it's likely we'd want to be able to use the EVE in a more general way, perhaps
 *	one code module would be using it for audio buffering, another for display interaction, etc.
 *  Writing our code around callbacks, Sming-style, it makes far more sense to buffer up these
 *  commands then execute them asynchronously at a higher level. The EVE driver interface will be
 *  at a high level, and internally will use SPI callbacks to deal with the transfers. Another source
 *  of wait-states are when the GPU or co-processor are busy with some other task. We should be able
 *  to make use of its interrupt signalling to deal with all that efficiently. It will need to
 *  incorporate a memory manager for the RAM so we can use it in a flexible manner.
 *
 */
void DisplayDemo::lcdMemCheck1()
{
	debug_i("Checking LCD memory...");

	constexpr unsigned bufSize = 512;
	uint32_t writeBuffer[2][bufSize / 4];
	uint32_t readBuffer[bufSize / 4];

	ElapseTimer timer;

	auto& stats = gfx.controller.stats;
	stats.clear();

	struct {
		uint32_t addr = 0;
		uint16_t bufIndex = 0;
		uint16_t count = 0;

		void next()
		{
			addr += bufSize;
			bufIndex = 1 - bufIndex;
			++count;
		}

		uint32_t endAddr()
		{
			return addr + bufSize - 1;
		}

	} write, read;

	HSPI::Request reqWr, reqRd;

	const uint32_t maxAddr = S1D13781_LUT1_BASE;
	while(true) {
		if(write.addr < maxAddr) {
			// LUT are 24-bit locations
			uint32_t mask = (write.addr < S1D13781_LUT1_BASE) ? 0xFFFFFFFF : 0xFFFFFF00;
			// NB. SPI read transaction taking place now (for row > 0)
			auto& writeBuf = writeBuffer[write.bufIndex];
			for(unsigned i = 0; i < ARRAY_SIZE(writeBuffer[0]); ++i) {
				writeBuf[i] = os_random() & mask;
			}
			gfx.write(reqWr, write.addr, writeBuf, bufSize);
			write.next();
		}

		if(write.count > 1) {
			while(reqRd.busy) {
				//
			}

			auto& writeBuf = writeBuffer[read.bufIndex];
			if(memcmp(readBuffer, writeBuf, bufSize) != 0) {
				debug_e("Mem check failed between 0x%08x and 0x%08x", read.addr, read.endAddr());

				for(unsigned i = 0; i < ARRAY_SIZE(readBuffer); ++i) {
					auto in = readBuffer[i];
					auto out = writeBuf[i];
					if(in != out) {
						debug_e("  @ 0x%08x: out 0x%08x in 0x%08x", read.addr + (i * 4), out, in);
						break;
					}
				}
			}

			read.next();
		}

		if(read.addr < maxAddr) {
			gfx.read(reqRd, read.addr, readBuffer, bufSize, [](HSPI::Request&) {});
		} else {
			break; // done
		}
	}

	debug_i("Memory check complete, %s, waitCycles = %u, trans = %u", timer.elapsedTime().toString().c_str(),
			stats.waitCycles, stats.transCount);
}

void DisplayDemo::test_vt100()
{
	/*
	 * Cursor tests
	 */
	static uint8_t state;
	uint8_t stateNumber = -1;

	auto width = terminal.width();
	auto height = terminal.height();

	STATE
	{
		display.initialise();
		terminal.reset();
		terminal.puts(_F("\e[?7h"));
		debug_i("Terminal initialised.");
		demoTimer.initializeMs<500>(TimerDelegate(&DisplayDemo::test_vt100, this)).start();
	}

	STATE
	{
		// clear screen
		terminal.puts(_F("\e[c\e[2J\e[m\e[r\e[?6l\e[1;1H"));
	}

	STATE
	{
		// draw a line of "*"
		terminal.putc('*', width);

		// draw left and right border
		for(unsigned line = 1; line < height; ++line) {
			terminal.printf(_F("\e[%u;1H*\e[%u;%uH*"), line, line, width);
		}

		// draw bottom line
		terminal.printf(_F("\e[%u;1H"), height);
		terminal.putc('*', width - 1);
	}

	STATE
	{
		// draw inner border of +
		terminal.puts(_F("\e[2;2H"));
		// draw a line of "*"
		terminal.putc('+', width - 2);

		// draw left and right border
		for(unsigned line = 1; line <= height; ++line) {
			terminal.printf(_F("\e[%u;2H+\e[%u;%uH+"), line, line, width - 1);
		}

		// draw bottom line
		terminal.printf(_F("\e[%d;2H"), height - 1);
		terminal.putc('+', width - 2);
	}

	STATE
	{
		// draw middle window
		// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
		// E                            E
		// E This must be an unbroken a E
		// E rea of text with 1 free bo E
		// E rder around the text.      E
		// E                            E
		// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
		terminal.puts(_F("\e[10;6H"));
		terminal.putc('E', 30);

		// test normal movement
		terminal.puts(_F("\e[11;6H"));

		// test cursor store and restore...
		terminal.puts(_F("\e7\e[35;10H\e8"));
		terminal.puts(_F("E\e[11;35HE"));

		// goto 12;6, print E, move cursor 29 (already moved +1) to right and print E
		terminal.puts(_F("\e[12;6HE\e[28CE"));

		// move cursor 31 to left, 1 down, print E, move 30 right, print E
		terminal.puts(_F("\e[30D\e[BE\e[28CE"));
		terminal.puts(_F("\e[15;6H\e[AE\e[28CE"));
		terminal.puts(_F("\e[15;6HE\e[15;35HE"));

		terminal.puts(_F("\e[16;6H"));
		terminal.putc('E', 30);
	}

	STATE
	{
		const char* text[] = {
			"This must be an unbroken a",
			"rea of text with 1 free bo",
			"rder around the text.     ",
		};
		for(unsigned c = 0; c < 3; c++) {
			terminal.printf(_F("\e[%u;8H"), c + 12);
			terminal.puts(text[c]);
		}
	}

	STATE
	{
		// now lets draw two parallel columns of Es
		terminal.puts(_F("\e[20;19H"));
		for(int c = 0; c < 10; c++) {
			// draw E (cursor moves right), step one right, draw F, step 3 left and 1 down
			terminal.puts(_F("E\e[1CF\e[3D\e[B"));
		}
	}

	// Test index (escD - down with scroll)
	// Test reverse index (escM)
	// next line (escE) - cr + index
	// save and restore cursor

	STATE
	{
		// move to last line and scroll down 7 lines
		terminal.printf("\e[%u;1H", width);
		for(int c = 0; c < 7; c++) {
			terminal.puts(_F("\eD"));
		}
	}

	STATE
	{
		// now scroll same number of lines back and then back again (to test up scroll)
		terminal.puts(_F("\e[1;1H"));
		for(int c = 0; c < 7; c++) {
			terminal.puts(_F("\eM"));
		}
	}

	STATE
	{
		terminal.printf("\e[%u;1H", width);
		for(int c = 0; c < 7; c++) {
			terminal.puts(_F("\eD"));
		}
	}

	STATE
	{
		// we now have the Es at the third line (or we SHOULD have)
		// refill the top border and clear bottom borders
		for(auto c = 1; c < width - 1; c++) {
			// we print * then move down and left, print + and go back right and top
			// (good way to test cursor navigation keys)
			terminal.printf(_F("\e[1;%dH*\e[B\e[D+\e[A"), c + 1);
		}
	}

	STATE
	{
		// clear the border that scrolled up
		for(auto c = 2; c < width - 2; c++) {
			// space, down, left, space, up
			terminal.printf(_F("\e[%u;%dH \e[B\e[D \e[A"), height - 8, c + 1);
		}
	}

	STATE
	{
		// redraw left and right border
		for(auto c = 1; c < height - 1; c++) {
			terminal.printf(_F("\e[%d;1H*+\e[%d;%dH+*"), c + 1, c + 1, width - 1);
		}
	}

	STATE
	{
		// fill border at the bottom
		for(int c = 0; c < width; c++) {
			terminal.printf(_F("\e[%u;%dH+\e[B\e[D*\e[A"), height - 1, c + 1);
		}
	}

	STATE
	{
		// draw the explanation string
		terminal.puts(_F("\e[30;6HShould see two columns of E F"));
		terminal.puts(_F("\e[31;6HText box must start at line 3"));
	}

	/*
	 * Tests setting scroll region and moving the cursor inside the scroll region.
	 */
	STATE
	{
		// reset terminal and clear screen. Cursor at 1;1.
		terminal.puts(_F("\e[c\e[2J\e[m\e[r\e[?6l\e[1;1H"));

		// set top margin 3 lines, bottom margin 5 lines
		terminal.puts(_F("\e[4;35r"));
	}

	STATE
	{
		// draw top and bottom windows
		terminal.puts(_F("\e[1;1H#\e[2;1H#\e[3;1H#\e[1;40H#\e[2;40H#\e[3;40H#"));
		terminal.puts(_F("\e[36;1H#\e[37;1H#\e[38;1H#\e[39;1H#\e[40;1H#"));
		terminal.puts(_F("\e[36;40H#\e[37;40H#\e[38;40H#\e[39;40H#\e[40;40H#"));
		terminal.puts(_F("\e[1;1H"));
		terminal.putc('#', 40);
		terminal.puts(_F("\e[3;1H"));
		terminal.putc('#', 40);
		terminal.puts(_F("\e[36;1H"));
		terminal.putc('#', 40);
		terminal.puts(_F("\e[40;1H"));
		terminal.putc('#', 40);
	}

	STATE
	{
		// print some text that should not move
		terminal.puts(_F("\e[2;4HThis is top text (should not move)"));
		terminal.puts(_F("\e[38;3HThis is bottom text (should not move)"));
	}

	STATE
	{
		// set origin mode and print border around the scroll region
		terminal.puts(_F("\e[?6h"));
		terminal.puts(_F("\e[1;1H"));
		terminal.putc('!', terminal.width());
	}

	STATE
	{
		// origin mode should snap 99 to last line in scroll region
		terminal.puts(_F("\e[99;1H"));
		terminal.putc('!', terminal.width());
	}

	STATE
	{
		for(unsigned line = 1; line <= terminal.height(); ++line) {
			//terminal.printf(_F("\e[%d;1H!\e[%d;%dH!", y, y, terminal.width());
			terminal.printf(_F("\e[%u;1H"), line);
			terminal.putc('!', terminal.width());
		}
	}

	STATE
	{
		// scroll the scroll region
		terminal.puts(_F("\e[99;1H\eD\eD"));
	}
	STATE
	{
		terminal.puts(_F("\e[1;1H\eM\eM"));
	}
	STATE
	{
		terminal.puts(_F("\e[99;1H\eD"));
	}

	STATE
	{
		// clear out an area in the middle and draw text
		for(int y = 0; y < 5; y++) {
			terminal.printf(_F("\e[%d;6H"), y + 10);
			terminal.putc(' ', 30);
		}
	}

	STATE
	{
		terminal.puts(_F("\e[11;10HMust be ! filled with 2"));
		terminal.puts(_F("\e[12;10H    empty lines at"));
		terminal.puts(_F("\e[13;10H    top and bottom! "));
	}

	STATE
	{
		LOAD_FSTR(text, flash_gltext);
		terminal.puts(text);

		state = 0;
		demoTimer
			.initializeMs<250>(([this]() {
				terminal.printf(_F("%u Timer callback...\n"), system_get_time());
				if(state++ == 10) {
					demoTimer.initializeMs<250>(TimerDelegate(&DisplayDemo::demoLoop, this)).startOnce();
				}
			}))
			.start(true);
	}

	++state;
}

void DisplayDemo::drawLogo()
{
	gfx.setColorDepth(Window::main, format_RGB_332LUT);
	gfx.setLutDefault(Window::main);
	gfx.fillWindow(Window::main, aclWhite);
	auto mainSize = gfx.getWindowSize(Window::main);
	unsigned imageWidth = 320;
	unsigned imageHeight = 320;
	unsigned x = (mainSize.width - imageWidth) / 2;
	unsigned y = (mainSize.height - imageHeight) / 4;
	gfx.drawImage(smingLogo, Window::main, x, y, imageWidth, imageHeight);

	y += imageHeight + y;
	SeFont font;
	font.load(fontTable[font_10x18]);
	String s = F("Let's do smart things!!!");
	x = (mainSize.width - font.getTextWidth(s.c_str())) / 2;
	gfx.drawText(Window::main, font, s.c_str(), x, y, 0, aclBlack, aclWhite, false);
}

void DisplayDemo::start()
{
	printDisplayConfig();

	lcdMemCheck1();

	debug_i("Checking LCD memory...");

	auto state = new MemCheckState(gfx);
	state->onComplete = [this]() {
		drawLogo();
		demoTimer.initializeMs<3000>(TimerDelegate(&DisplayDemo::test_vt100, this)).startOnce();
	};
	state->execute();
}

void DisplayDemo::stop()
{
	demoTimer.stop();
}
