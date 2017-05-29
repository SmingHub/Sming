//*****************************************************************************
// image_example
//
// Example for Image loader running Sming
// on the ESP8266
//
// Public domain, Sebastien Leclerc 2016
//
// This application example will attempt to load all images listed in a
// string array from the spiffy file system and display them as pages
// of tiles with random images.
//
// The images are all 16x16 so many of them fit nice on a small screen.
//
//*****************************************************************************

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <Libraries/image/image.h>
#include "application.h"
#include <SmingCore/Debug.h>

// example is using I2C
#define SDA_PIN 13
#define SCL_PIN 12

Vector<image_struct *> imageList;
Adafruit_SSD1306 display(16);
Timer refresh_timer;

//*****************************************************************************
// Initialize hardware and load images
//*****************************************************************************
//*****************************************************************************
// Load the images buffers in memory so that decoding is only done once
//*****************************************************************************
void loadImages() {
	debugf("Loading images");
	uint32 curFreeHeap = system_get_free_heap_size();

	for (int i = 0; imageFileNameList[i] != NULL ; i++) {
		// Req Channels:  	0 to retrieve all color channels from picture
		// 					'1'..'4' to force that many components per pixel
		image_struct * pnewImage = image_load(imageFileNameList[i], 1);
		if (!pnewImage) {
			debugf("Could not load %s", imageFileNameList[i].c_str());
		} else {
			imageList.add(pnewImage);
			DumpAscii(pnewImage);
			debugf("Loaded %s", imageFileNameList[i].c_str());
		}
	}
	debugf("Images Loaded. Memory taken from Heap %i, Free heap: %i\n\nList of image pointers: ",
			curFreeHeap - system_get_free_heap_size(),
			system_get_free_heap_size());
//	image_dump_pointers();

}

//*****************************************************************************
// Schedule a timer event to draw random tiles
//*****************************************************************************
void startWork() {
	refresh_timer.initializeMs(1500, DrawPage).start();
}

//*****************************************************************************
// Draw a page full of icons.
//*****************************************************************************
void DrawPage() {
//	uint32 curFreeHeap = system_get_free_heap_size();

	// Get a list of random tiles
	Vector<tile_struct *> NewTileList = buildRandomTileList();

	// Clear display
	debugf("drawing page. Tile count is %u.",NewTileList.count());
	display.clearDisplay();
	display.setTextColor(BLACK);
	display.fillRect(0, 0, 132, 64, BLACK);

	// Display all tiles
	for (int i = 0; i < NewTileList.count(); i++) {
		tile_struct * tile = NewTileList[i];

		if (!DrawImage(tile->image, tile->pos->X, tile->pos->Y)) {
			debugf("Could not draw image %s ",
					tile->image->fileName.c_str());
		}

		STBI_FREE(tile);
	}

	// display and cleanup

	display.display();
	NewTileList.removeAllElements();

	// In case some memory issue needs to be debugged
//	debugf("Page Drawn. Memory taken from Heap %i (should be close to zero), Free heap: %i",
//			curFreeHeap - system_get_free_heap_size(),
//			system_get_free_heap_size());
}

//*****************************************************************************
//
// Build a list of tiles from random images in memory. The tiles positions are
// defined in the application.h file.
//
//*****************************************************************************

Vector<tile_struct *> buildRandomTileList() {
	Vector<tile_struct *> NewTileList;
	//debugf("Building single page");
//	uint32 curFreeHeap = system_get_free_heap_size();

	for (int j = 0; screenTiles[j].X != -1; j++) {
		int r = random(0, imageList.count() - 1);
		tile_struct * pnewTile = (tile_struct *) STBI_MALLOC(sizeof(tile_struct));
		if (pnewTile) {
			pnewTile->pos = &screenTiles[j];
			pnewTile->image = imageList[r];
			NewTileList.add(pnewTile);
		} else {
			debugf("Could not allocate memory for tile %u of page", j);
		}
	}

//	debugf("Page built. Memory taken from Heap %i, Free heap: %i",
//			curFreeHeap - system_get_free_heap_size(),
//			system_get_free_heap_size());
	return NewTileList;

}

//*****************************************************************************
//
// Take the raw content of the uncompressed image buffer and display it on
// Screen.
//
//*****************************************************************************
bool  DrawImage(image_struct * data, int xPos, int yPos) {
	unsigned char *pCurdata;
	int16_t height = 0, width = 0, curYPos = 0, curXPos = 0;
	height = display.height();
	width = display.width();
	if (data) {
		pCurdata = data->imagedata;

		//    // ... process data if not NULL ...
		for (int curY = 0; curY < data->height; curY++) {
			for (int curX = 0; curX < data->width; curX++) {
				curXPos = curX + xPos;
				curYPos = curY + yPos;
				if (height >= curYPos and width >= curXPos) {
					// crude way of converting gray scale to black and white.
					// jpgs were chosen to still appear nicely
					display.drawPixel(curXPos, curYPos, *pCurdata > 250 ? WHITE : BLACK );
				}

				pCurdata += 1;
			}
		}

	} else {
		debugf("Could not draw image at [%u,%u]",xPos, yPos);
		return false;
	}

	return true;

}

bool  DumpAscii(image_struct * data) {
	unsigned char *pCurdata;
	int16_t curYPos = 0, curXPos = 0;
	if (data) {
		pCurdata = data->imagedata;
		//    // ... process data if not NULL ...
		for (int curY = 0; curY < data->height; curY++) {
			for (int curX = 0; curX < data->width; curX++) {
					Serial.print(*pCurdata>250 ? ' ' : '@' );
				pCurdata += 1;
			}
			Serial.println("");
		}

	} else {
		debugf("Could not dump image");

		return false;
	}

	return true;

}
void  init() {
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Debug.setDebug(Serial);
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	Wire.pins(SCL_PIN, SDA_PIN);
	display.begin(SSD1306_SWITCHCAPVCC);
	display.setTextColor(BLACK);
	display.fillScreen(WHITE);
	display.display();
	display.setTextSize(1);
	display.printf("Loading");
	display.display();
//	display.drawPixel(1, 1,  WHITE );

	loadImages();
	System.onReady(startWork);

}

