//*****************************************************************************
// image_example
//
// Example for Image loader running Sming
// on the ESP8266
//
// Public domain, Sebastien Leclerc 2016
//
//*****************************************************************************


#ifndef INCLUDE_APPLICATION_H_
#define INCLUDE_APPLICATION_H_
typedef struct {
	int Y;
	int X;
} coordinates_struct ;

typedef struct {
	coordinates_struct * pos;
	image_struct * image;
} tile_struct;


// some example images taken from http://www.iconsplace.com/
// Do not attempt to load them all...  unless there is enough memory on
// the processor you are building for
String imageFileNameList[] = {
		"addressbook.gif",
		"alert.gif",
		"arrow2_ne.gif",
		"arrow2_n.gif",
		"arrow2_nw.gif",
		"back.gif",
		"check.gif",
		"close.gif",
		"download.gif",
		"home.gif",
		"lock.gif",
		"search.gif",
		"stop.gif",
		NULL
};


// Here we define all the screen tiles coordinates
// Below are 16x16 tiles
coordinates_struct screenTiles[] = {
		{0,0},{0,16},{0,32},{0,48},{0,64},{0,80},{0,96},{0,112},{0,128},{0,144},{0,160},
		{16,0},{16,16},{16,32},{16,48},{16,64},{16,80},{16,96},{16,112},{16,128},{16,144},{16,160},
		{32,0},{32,16},{32,32},{32,48},{32,64},{32,80},{32,96},{32,112},{32,128},{32,144},{32,160},
		{48,0},{48,16},{48,32},{48,48},{48,64},{48,80},{48,96},{48,112},{48,128},{48,144},{48,160},{-1,-1}};

void DrawPage();
void startWork();
void loadImages();
bool DrawImage(image_struct * data, int xPos, int yPos);
Vector<tile_struct *>  buildRandomTileList() ;
bool DumpAscii(image_struct * data);
void init();
bool image_draw(unsigned char * data, int xPos, int yPos);
#endif
