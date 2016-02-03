#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <Libraries/image/image.h>
#include "application.h"

// example is using I2C
#define SDA_PIN 13
#define SCL_PIN 12

#define SCREEN_COUNT 3

Vector<image_struct *> imageList;
Vector<page_struct *> pages;

Adafruit_SSD1306 display(16);
Timer refresh_timer;
int page = 0;


unsigned long elapse_start = micros();
#define START_ELAPSED(operation) { elapse_start = micros(); Serial.printf("Begin operation %s",operation); Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size()); 	system_print_meminfo();}
#define END_ELAPSED(operation,detail) { unsigned long end = micros(); unsigned long delta = end - elapse_start;  Serial.printf("%s/%s rendering time: %i\n",operation,detail ,delta); Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size()); 	system_print_meminfo();}
//#define START_ELAPSED(operation) ;
//#define END_ELAPSED(operation,detail) ;

void drawPixelDelegate(int x, int y, int color) {
	if(display.height()>=y and display.width() >= x ) {
		// crude way of converting gray scale to black and white.
		// jpgs were chosen to still appear nicely
		display.drawPixel(x,y,color>254?BLACK:WHITE);
	}

}
void loadImages(){
	Serial.println("Loading images");
	uint32 curFreeHeap = system_get_free_heap_size();

	for(int i=0;i<FILENAME_COUNT;i++)
		for(int j=0;j<IMAGE_TYPES_COUNT;j++){

			// load images, with 1 component per pixel
			String fileName = imageFileNameList[i]+"."+imageTypesList[j];
			image_struct * pnewImage = image_load(fileName,1);
			if(!pnewImage){
				Serial.print("Could not load ");
				Serial.println(fileName.c_str());
			}
			else
			{
				imageList.add(pnewImage);
			}
		}
//	Serial.print("Loaded images. Memory taken from Heap ");
//	Serial.print(curFreeHeap-system_get_free_heap_size());
//	Serial.print(" Free heap: ");
//	Serial.println(system_get_free_heap_size());
//	Serial.println("===	system_print_meminfo ===");
//	system_print_meminfo();
	image_dump_pointers();
	Serial.print("Done loading images\n");
//	buildpages_timer.initializeMs(1500, buildPages).startOnce();

}
void buildPages(){
	Serial.println("Building pages");
	uint32 curFreeHeap = system_get_free_heap_size();
	for(int i=0;i<SCREEN_COUNT;i++){
		page_struct * pNewPage = new page_struct;
		if(!pNewPage){
			Serial.printf("failed to allocate page structure for screen %i",i+1);
			return;
		}
		pNewPage->pageNo= i+1;
		for(int j=0;j<TILE_COUNT;j++){
			int r=random(0, imageList.count()-1);
			tile_struct * pnewTile = (tile_struct *)STBI_MALLOC(sizeof(tile_struct));
			pnewTile->pos = &screenTiles[j];
			pnewTile->image = imageList[r];
			pNewPage->tile.add(pnewTile);

		}
		pages.add(pNewPage);
	}
	Serial.printf("Pages built. Memory taken from Heap %i, Free heap: %i\n", system_get_free_heap_size()-curFreeHeap,system_get_free_heap_size());

}

bool image_draw(image_struct * data, int xPos, int yPos){
	unsigned char *pCurdata;

	if(data){
		pCurdata = data->imagedata;
		//    // ... process data if not NULL ...
			for(int curY=0;curY<data->height;curY++){
				for(int curX=0;curX<data->width;curX++){
				// ... x = width, y = height, n = # 8-bit components per pixel ...
				//    // ... replace '0' with '1'..'4' to force that many components per pixel
				//    // ... but 'n' will always be the number that it would have been if you said 0
 				drawPixelDelegate(curX+xPos,curY+yPos,*pCurdata);
 				pCurdata+=1;
			}
		}
	}
	else
	{
		return false;
	}

	return true;

}
void DrawPage(){
	uint32 curFreeHeap = system_get_free_heap_size();
	if(page>=pages.count()) page = 0;
	display.clearDisplay();
	display.setTextColor(BLACK);
	display.fillRect(0,0,132,64,BLACK);
	for(int i=0;i<pages[page]->tile.count();i++){
		image_draw(pages[page]->tile[i]->image,pages[page]->tile[i]->pos->X,pages[page]->tile[i]->pos->Y);
	}
	display.display();
	page++;
}

void startWork() {
	refresh_timer.initializeMs(1500, DrawPage).start();
}
void init() {
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	Wire.pins(SCL_PIN, SDA_PIN);
	display.begin(SSD1306_SWITCHCAPVCC);
	display.setTextColor(BLACK);
	display.fillScreen(BLACK);
	display.display();
	display.setTextSize(1);
	display.printf("Loading");
	display.display();
	loadImages();
	buildPages();

	page=0;
	System.onReady(startWork);

}
