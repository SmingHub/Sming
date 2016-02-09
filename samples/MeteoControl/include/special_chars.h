#ifndef INCLUDE_SPECIAL_CHARS_H_
#define INCLUDE_SPECIAL_CHARS_H_

uint8_t celsius[8] =
{
	B00111,
	B00101,
	B00111,
	B00000,
	B00000,
	B00000,
	B00000
};

byte icon_termometer[8] = //icon for termometer
{
    B00100,
    B01010,
    B01010,
    B01110,
    B01110,
    B11111,
    B11111,
    B01110
};

byte icon_water[8] = //icon for water droplet
{
    B00100,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B10001,
    B01110,
};

uint8_t icon_check[8] = {0x0, 0x1 ,0x3, 0x16, 0x1c, 0x8, 0x0};
uint8_t icon_cross[8] = {0x0, 0x1b, 0xe, 0x4, 0xe, 0x1b, 0x0};
uint8_t icon_bell[8]  = {0x4, 0xe, 0xe, 0xe, 0x1f, 0x0, 0x4};
uint8_t icon_retarrow[8] = {	0x1, 0x1, 0x5, 0x9, 0x1f, 0x8, 0x4};
uint8_t icon_clock[8] = {0x0, 0xe, 0x15, 0x17, 0x11, 0xe, 0x0};

#endif /* INCLUDE_SPECIAL_CHARS_H_ */
