#include <SmingCore.h>
#include <AnimatedGIF.h>
#include <Graphics/SampleConfig.h>

namespace
{
AnimatedGIF gif;
Timer playTimer;
Graphics::Surface* surface;

IMPORT_FSTR(gifData, PROJECT_DIR "/files/frog.gif")

void draw(GIFDRAW* pDraw)
{
	const auto& tftSize = tft.getSize();
	const int DISPLAY_WIDTH = tftSize.w;
	const int DISPLAY_HEIGHT = tftSize.w;

	auto pixelFormat = tft.getPixelFormat();
	auto bytesPerPixel = Graphics::getBytesPerPixel(pixelFormat);

	uint16_t usTemp[DISPLAY_WIDTH];
	Graphics::SharedBuffer buffer(bytesPerPixel * DISPLAY_WIDTH);

	int iWidth = pDraw->iWidth;
	if(iWidth + pDraw->iX > DISPLAY_WIDTH) {
		iWidth = DISPLAY_WIDTH - pDraw->iX;
	}
	const uint16_t* usPalette = pDraw->pPalette;
	int y = pDraw->iY + pDraw->y; // current line
	if(y >= DISPLAY_HEIGHT || pDraw->iX >= DISPLAY_WIDTH || iWidth < 1) {
		return;
	}

	auto s = pDraw->pPixels;
	if(pDraw->ucDisposalMethod == 2) // restore to background color
	{
		for(int x = 0; x < iWidth; x++) {
			if(s[x] == pDraw->ucTransparent) {
				s[x] = pDraw->ucBackground;
			}
		}
		pDraw->ucHasTransparency = 0;
	}

	// Apply the new pixels to the main image
	if(pDraw->ucHasTransparency) // if transparency used
	{
		uint8_t ucTransparent = pDraw->ucTransparent;
		uint8_t* pEnd = s + iWidth;
		int x = 0;
		int iCount = 0; // count non-transparent pixels
		while(x < iWidth) {
			uint8_t c = ucTransparent - 1;
			uint16_t* d = usTemp;
			while(c != ucTransparent && s < pEnd) {
				c = *s++;
				if(c == ucTransparent) {
					// done, stop: back up to treat it like transparent
					s--;
				} else {
					// opaque
					*d++ = __builtin_bswap16(usPalette[c]);
					iCount++;
				}
			}				// while looking for opaque pixels
			if(iCount != 0) // any opaque pixels?
			{
				Graphics::convert(usTemp, Graphics::PixelFormat::RGB565, buffer.get(), pixelFormat, iCount);
				Graphics::Rect r(pDraw->iX + x, y, iCount, 1);
				surface->reset();
				surface->setAddrWindow(r);
				surface->writeDataBuffer(buffer, 0, iCount * bytesPerPixel);
				surface->present();
				x += iCount;
				iCount = 0;
			}
			// no, look for a run of transparent pixels
			c = ucTransparent;
			while(c == ucTransparent && s < pEnd) {
				c = *s++;
				if(c == ucTransparent) {
					iCount++;
				} else {
					s--;
				}
			}
			if(iCount != 0) {
				// skip these
				x += iCount;
				iCount = 0;
			}
		}
	} else {
		// No transparency
		s = pDraw->pPixels;
		// Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
		for(int x = 0; x < iWidth; x++) {
			usTemp[x] = __builtin_bswap16(usPalette[*s++]);
		}
		Graphics::convert(usTemp, Graphics::PixelFormat::RGB565, buffer.get(), tft.getPixelFormat(), iWidth);
		Graphics::Rect r(pDraw->iX, y, iWidth, 1);
		surface->reset();
		surface->setAddrWindow(r);
		surface->writeDataBuffer(buffer, 0, iWidth * bytesPerPixel);
		surface->present();
	}
}

void nextFrame()
{
	gif.playFrame(true, nullptr);
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	initDisplay();
	tft.setOrientation(Graphics::Orientation::deg270);
	surface = tft.createSurface();

	gif.openFLASH((uint8_t*)gifData.data(), gifData.length(), draw);

	playTimer.initializeMs(500, nextFrame).start();
}
