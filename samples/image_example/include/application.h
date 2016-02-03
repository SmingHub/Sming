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

typedef struct {
	int pageNo;
	Vector<tile_struct *> tile;
} page_struct;
// some example images taken from http://www.iconsplace.com/
#define FILENAME_COUNT 10
String imageFileNameList[FILENAME_COUNT] = {
		"black-key-16",
		"black-restriction-shield-16",
		"black-warning-shield-16",
		"black-delete-shield-16",
		"black-question-shield-16",
		"black-shield-16",
		"black-web-shield-16",
		"black-firewall-16",
		"black-refresh-shield-16",
		"black-user-shield-16"};
#define IMAGE_TYPES_COUNT 1
String imageTypesList[IMAGE_TYPES_COUNT] = {"jpg"};//,"png"};
#define TILE_COUNT 44
coordinates_struct screenTiles[TILE_COUNT] = {
		{0,0},{0,16},{0,32},{0,48},{0,64},{0,80},{0,96},{0,112},{0,128},{0,144},{0,160},
		{16,0},{16,16},{16,32},{16,48},{16,64},{16,80},{16,96},{16,112},{16,128},{16,144},{16,160},
		{32,0},{32,16},{32,32},{32,48},{32,64},{32,80},{32,96},{32,112},{32,128},{32,144},{32,160},
		{48,0},{48,16},{48,32},{48,48},{48,64},{48,80},{48,96},{48,112},{48,128},{48,144},{48,160}};


void DrawPage();
void startWork();
bool image_draw(image_struct * data, int xPos, int yPos);

void init();
bool image_draw(unsigned char * data, int xPos, int yPos);
#endif
