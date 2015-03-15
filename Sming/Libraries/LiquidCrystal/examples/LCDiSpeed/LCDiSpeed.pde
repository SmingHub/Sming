static char dummyvar; // dummy declaration for STUPID IDE!!!!
/*----------------------------------------------------------------------------
 * vi:ts=4
 *
 * LCDiSpeed - LCD Interface Speed
 *
 * Created by Bill Perry 2012-03-16
 * Copyright 2012 - Under creative commons license 3.0:
 * Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 * license page: http://creativecommons.org/licenses/by-nc-sa/3.0/
 *
 * Sketch to measure and report the speed of the interface to the LCD and
 * speed of updating the LCD.
 *
 * It runs a Frames/Sec (FPS) test which writes a "frame" of each digit 0-9 to
 * the display.
 * A "frame" is a full display of characters.
 * It is created by positioning the cursor to the begining of each row
 * and then writing a character to every position on the row, until the
 * entire display is filled.
 * The FPS test does a frame of 9's then 8's, .... down to 0's
 * On fast interfaces it will not normally be seen.
 *
 * The sketch will then calculate & report transfer speeds and
 * LCD update rates to the LCD display.
 *
 * Reported Information:
 * - Single byte transfer speed (ByteXfer)
 *		This is the time it takes for a single character to be sent from
 *		the sketch to the LCD display.
 *
 * - Frame/Sec (FPS)
 *		This is the number of times the full display can be updated
 *		in one second. 
 *     
 * - Frame Time (Ftime)
 *		This is the amount of time it takes to update the full LCD display.
 *
 *
 * The sketch will also report "independent" FPS and Ftime values.
 * These are timing values that are independent of the size of the LCD under test.
 * Currently they represent the timing for a 16x2 LCD
 * The value of always having numbers for a 16x2 display
 * is that these numbers can be compared to each other since they are
 * independent of the size of the actual LCD display that is running the test.
 *
 * All times & rates are measured and calculeted from what a sketch "sees"
 * using the LiquidCrystal API.
 * It includes any/all s/w overhead including the time to go through the
 * Arduino Print class and LCD library.
 * The actual low level hardware times are obviously lower.
 *
 * History
 * 2012.03.15 bperrybap - Original creation
 *
 * @author Bill Perry - bperrybap@opensource.billsworld.billandterrie.com
 *----------------------------------------------------------------------------

/*
 * Define your LCD size
 */
#define LCD_COLS 16
#define LCD_ROWS 2

/*
 * Pick your interface.
 */

//#define LCDIF_4BIT
//#define LCDIF_I2C
//#define LCDIF_SR2W
//#define LCDIF_SR_2W // SR in 2 wire mode
//#define LCDIF_SR_3W // SR in 3 wire mode
#define LCDIF_SR3W
//#define LCDIF_SR1W


/*
 * Options
 */

#define FPS_iter 1 	// number of iterations to repeat each "frame" within the test 
					// (current frame test is 10 full display frames, 1 for each digits 0-9)
					// FPS_iter like 100 will allow the frames to be seen
					// Note: the only reason other than visual to make this larger than 1
					// might be to compensate for Async serial buffering should a serial interface be tested
					// even with 1 iteration, 340 bytes are written for a 16x2 display
					// bytes written = FPS_iter * ((LCD_ROWS * LCD_COLS) + LCD_ROWS) * 10

#define iLCD		// turn on code to calculate speed of "independent" sized display
#define iLCD_ROWS 2 // independent FPS row size
#define iLCD_COLS 16 // independent FPS col size


#define DELAY_TIME 3500 // delay time to see information on lcd

#if defined(LCDIF_4BIT)

// Include the Liquid Crystal library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
//            lcd(RS,  E, d4, d5, d6, d7, bl, polarity)
#ifdef BACKLIGHT_ON
LiquidCrystal lcd( 8,  9,  4,  5,  6,  7, 10, POSITIVE); // new constructor with backlight support
#else
LiquidCrystal lcd( 8,  9,  4,  5,  6,  7); // old style constructor w/o backlight (to test old library)
#endif

#elif defined(LCDIF_I2C)
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x38);  // Set the LCD I2C address


#elif defined(LCDIF_SR3W)
#include <LiquidCrystal_SR3W.h>
                 //     d,clk,strb, en,rw,rs,d4,d5,d6,d7,bl,blpol
//LiquidCrystal_SR3W lcd (2, 3,  4,    6, 7, 1, 2, 3, 4, 5, 0, POSITIVE);
LiquidCrystal_SR3W lcd(3, 2, 4);

#elif defined(LCDIF_SR2W)

#include <LiquidCrystal_SR2W.h>
                 //   d, clk, (blPolarity optional, defaults to POSITIVE)
LiquidCrystal_SR2W lcd (2, 3);

#elif defined(LCDIF_SR1W)

#include <LiquidCrystal_SR1W.h>
LiquidCrystal_SR1W lcd (2);

#elif defined(LCDIF_SR_2W)

#include <LiquidCrystal_SR.h>
                 //   d, clk
LiquidCrystal_SR lcd (2, 3);

#elif defined(LCDIF_SR_3W)

#include <LiquidCrystal_SR.h>
                 //   d,clk,strb
LiquidCrystal_SR lcd (2, 3,  4);

#endif

void setup()
{
	// set up the LCD's number of columns and rows: 
	lcd.begin(LCD_COLS, LCD_ROWS);

#ifdef BACKLIGHT_ON
	lcd.backlight(); // make sure backlight is on with new library
#endif
}

void loop()
{
unsigned long etime;
char buf[8];

	lcd.clear();


	/*
	 * Time an FPS test
	 */

	etime = timeFPS(FPS_iter, LCD_COLS, LCD_ROWS);

	/*
 	 * show the average single byte xfer time during the FPS test
	 */
	showByteXfer(etime);

	/*
	 * show FPS rate and Frame update time for this display
	 */

	sprintf(buf, "%dx%d", LCD_COLS, LCD_ROWS);
	showFPS(etime, buf);

#ifdef iLCD
	/*
	 * calculate Independent FPS and Frame update time
	 * (rate & time for a "standard" display - default of 16x2)
	 * This is simply a matter of scaling the time based on the
	 * ratio of the display sizes.
	 */

	etime = etime *iLCD_ROWS * iLCD_COLS / LCD_ROWS / LCD_COLS;

	/*
	 * show independent FPS rate & Frame update time
	 */
	sprintf(buf, "%dx%d", iLCD_COLS, iLCD_ROWS);
	showFPS(etime, buf);
#endif

}

unsigned long timeFPS(uint8_t iter, uint8_t cols, uint8_t rows)
{
char c;
unsigned long stime, etime;

	stime = micros();
	for(c = '9'; c >= '0'; c--) // do not change this unless you change the FPS/ByteXfer calcuations as well
	{
		for(uint8_t i = 0; i < iter; i++)
		{
			for(uint8_t row = 0; row < rows; row++)
			{
				lcd.setCursor(0, row);
				for(uint8_t col = 0; col< cols;col++)
				{
					lcd.write(c);
				}
			}
		}
	}
	etime = micros();
	return((etime-stime));
}
void showFPS(unsigned long etime, const char *type)
{
float fps;


	/*
	 * calculate Frame update time and FPS rate
	 * The 10.0 is for the 10 frames done per iteration
	 * one for each digit 0-9
	 */

	fps = (10.0 * FPS_iter) * 1000000.0/(etime);


	lcd.clear();
	lcd.print(type);
	lcd.print("FPS: ");
	lcd.print(fps);

	if(LCD_ROWS > 1)
	{
		lcd.setCursor(0,1);
	}
	else
	{
		delay(DELAY_TIME);
		lcd.clear();
	}
	lcd.print("Ftime: ");
	lcd.print((etime)/10.0/FPS_iter/1000);
	lcd.print("ms");

	delay(DELAY_TIME);
}
void showByteXfer(unsigned long etime)
{
	lcd.clear();
	lcd.print("ByteXfer: ");
	/*
	 * Calculate average byte xfer time from time of FPS test
	 * This takes into consideration the set cursor position commands which
	 * are single byte commands and take the same amount of time as a data byte write.
	 * The final result is rounded up to an integer.
	 */
	lcd.print((int) (etime / (FPS_iter * (10.0 * (LCD_COLS *  LCD_ROWS + LCD_ROWS)))+0.5));
	lcd.print("uS");

	delay(DELAY_TIME);
}
