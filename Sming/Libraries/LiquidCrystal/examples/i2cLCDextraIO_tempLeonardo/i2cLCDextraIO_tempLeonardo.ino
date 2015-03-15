// ---------------------------------------------------------------------------
// Created by Francisco Malpartida on 1/1/12.
// Copyright 2011 - Under creative commons license:
//        Attribution-NonCommercial-ShareAlike CC BY-NC-SA
//
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file i2CLCDextraIO_tempLeonardo.ino
// Temperature logging to demonstrate the I2CLCDextraIO library.
// 
// @brief This application is a demostration file for the I2CLCDextraIO library
// that reads a temperature from the internal ATMEGA32U4 temperature sensor
// and displays it on the LCD. The application also demonstrates some of the
// methods of the library, such as loading custom characters to the LCD,
// moving around the LCD, and writing to it.
//
// @author F. Malpartida
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <Wire.h>
#include <LCD.h>

#define _LCD_SR3W_

#ifdef _LCD_I2C_
#include <LiquidCrystal_I2C.h>
#endif

#ifdef _LCD_4BIT_
#include <LiquidCrystal.h>
#endif


#ifdef _LCD_SR_
#include <LiquidCrystal_SR.h>
#endif

#ifdef _LCD_SR3W_
#include <LiquidCrystal_SR3W.h>
#endif


/*!
    @defined    CHAR_WIDTH
    @abstract   Character witdth of the display, expressed in pixeles per character.
*/
#define CHAR_WIDTH  5

/*!
    @defined    BACKLIGHT_PIN
    @abstract   LCD backlight pin definition.
    @discussion AVR pin used for the backlight illumintation of the LCD.
*/
#define BACKLIGHT_PIN          12

/*!
    @defined    STATUS_PIN
    @abstract   Status LED indicator.
    @discussion Activity LED blinking indicating that the system is up.
*/
#define STATUS_PIN             13

/*!
    @defined    LOOP_DELAY
    @abstract   Main loop delay.
    @discussion Main loop delay executing temperature readings and LCD updates.
*/
#define LOOP_DELAY            300

/*!
    @defined    TEMP_CAL_OFFSET
    @abstract   Temperature calibration offset.
    @discussion This is the offset value that has to be modified to get a
                correct temperature reading from the internal temperature sensor
                of your AVR.
*/
#define TEMP_CAL_OFFSET 282

/*!
    @defined    FILTER_ALP
    @abstract   Low pass filter alpha value
    @discussion This value defines how much does the current reading, influences
                the over all value. The smaller, the less influence the current
                reading has over the overall result.
*/
#define FILTER_ALP 0.1

/*!
    @defined    MIN_TEMP
    @abstract   Minimum temperature range for bargraph
 
*/
#define MIN_TEMP   -10

/*!
    @defined    MAX_TEMP
    @abstract   Maximum temperature range for bargraph
 
*/
#define MAX_TEMP   50

extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;


// Initialise LCD module
// -----------------------------------------------------------------------------
#ifdef _LCD_I2C_
LiquidCrystal_I2C lcd(0x38);
#endif

#ifdef _LCD_4BIT_
LiquidCrystal lcd(12, 11, 5, 4, 3, 2, BACKLIGHT_PIN, POSITIVE);
const int    CONTRAST_PIN  = 9;
const int    CONTRAST      = 65;
#endif

#ifdef _LCD_SR_
LiquidCrystal_SR lcd(3,2,TWO_WIRE);
//                   | |
//                   | \-- Clock Pin
//                   \---- Data/Enable Pin
#endif

#ifdef _LCD_SR3W_
LiquidCrystal_SR3W lcd(3, 2, 4);
//                     | |
//                     | \-- Clock Pin
//                     \---- Data/Enable Pin
#endif


// LCD reference variable
LCD *myLCD = &lcd;

// Temperature filter variable
static double tempFilter;

/*!
    @const      charBitmap 
    @abstract   Define Character bitmap for the bargraph.
    @discussion Defines a character bitmap to represent a bargraph on a text
    display. The bitmap goes from a blank character to full black.
*/
const uint8_t charBitmap[][8] = {
   { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },
   { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0 },
   { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x0 },
   { 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x0 },
   { 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x0 },
   { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0 },
   { 0xe, 0x11, 0x11, 0x11, 0xe, 0, 0, 0 },
   { 0x6, 0x9, 0x9, 0x6, 0x0, 0, 0, 0}
};

/*!
    @function
    @abstract   Return available RAM memory
    @discussion This routine returns the ammount of RAM memory available after
                initialising the C runtime.
    @param      
    @result     Free RAM available.
*/
static int freeMemory() 
{
  int free_memory;

  if((int)__brkval == 0)
     free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);

  return free_memory;
}

/*!
    @function
    @abstract   Returns AVR328p internal temperature
    @discussion Configures the ADC MUX for the temperature ADC channel and
                waits for conversion and returns the value of the ADC module
    @result     The internal temperature reading - in degrees C 
*/

static int readTemperature()
{
   ADMUX = 0xC7;                          // activate interal temperature sensor, 
                                          // using 2.56V ref. voltage
   ADCSRB |= _BV(MUX5);
   
   ADCSRA |= _BV(ADSC);                   // start the conversion
   while (bit_is_set(ADCSRA, ADSC));      // ADSC is cleared when the conversion 
                                          // finishes
                                          
   // combine bytes & correct for temperature offset (approximate)
   return ( (ADCL | (ADCH << 8)) - TEMP_CAL_OFFSET);  
}

/*!
    @function
    @abstract   Braws a bargraph onto the display representing the value passed.
    @discussion Draws a bargraph on the specified row using barLength characters. 
    @param      value[in] Value to represent in the bargraph
    @param      row[in] Row of the LCD where to display the bargraph. Range (0, 1)
                for this display.
    @param      barlength[in] Length of the bar, expressed in display characters.
    @param      start[in]     Start bar character
    @param      end [in]      End bar character

    @result     None
*/
static void drawBars ( int value, uint8_t row, uint8_t barLength, char start, 
                       char end )
{
   int numBars;

   // Set initial titles on the display
   myLCD->setCursor (0, row);
   myLCD->print (start);

   // Calculate the size of the bar
   value = map ( value, MIN_TEMP, MAX_TEMP, 0, ( barLength ) * CHAR_WIDTH );
   numBars = value / CHAR_WIDTH;
   
   // Limit the size of the bargraph to barLength
   if ( numBars > barLength )
   {
     numBars = barLength;
   }
   myLCD->setCursor ( 1, row );
   
   // Draw the bars
   while ( numBars-- )
   {
      myLCD->print ( char( 5 ) );
   }
   
   // Draw the fractions
   numBars = value % CHAR_WIDTH;
   myLCD->print ( char(numBars) );
   myLCD->setCursor (barLength + 1, row);
   myLCD->print (end);

}

/*!
    @function
    @abstract   Initialise the HW
    @discussion Initialise the HW used within this application: UART, LCD & IOs
    @param      
    @result     
*/

static void initHW ( void )
{
   int i; 
   int charBitmapSize = (sizeof(charBitmap ) / sizeof (charBitmap[0]));
   
   Serial.begin ( 57600 );
   
   // Hardware initialise
   // ------------------------------------
   
   //ADCSRA |= (1 << ADEN);  // Initialise ADC block (no need done by env)
   
   // Initialise LCD HW: backlight and LCD
   // -------------------------------------
#ifdef _LCD_4BIT_
  pinMode(CONTRAST_PIN, OUTPUT);
  analogWrite (CONTRAST_PIN, CONTRAST);
#endif

#ifdef _LCD_I2C_
   pinMode ( BACKLIGHT_PIN, OUTPUT );
   digitalWrite (BACKLIGHT_PIN, HIGH);
#endif
   pinMode ( STATUS_PIN, OUTPUT );

   myLCD->begin ( 20, 2 );
   // Load custom character set into CGRAM
   // --------------------------------------------------------------------
   for ( i = 0; i < charBitmapSize; i++ )
   {
      myLCD->createChar ( i, (uint8_t *)charBitmap[i] );
   }
}

void setup ()
{
   initHW();
   
   Serial.println ( freeMemory () );
   myLCD->clear ();
   myLCD->print ( F("Free Mem: "));
   myLCD->print ( freeMemory () );
   delay ( 2000 );
   myLCD->clear ();
   myLCD->print (F("Temp:"));
   myLCD->setCursor ( 8, 0 );

   tempFilter = 0;
   myLCD->print ( readTemperature() );
}


void loop ()
{
  int temp;
  static byte status = 1;
  
  status ^= 1;
  digitalWrite ( STATUS_PIN, status);
  
  temp = readTemperature();
  tempFilter = ( FILTER_ALP * temp) + (( 1.0 - FILTER_ALP ) * tempFilter);

  // Display the information to the LCD
  myLCD->setCursor ( 8, 0 );
  myLCD->print ("     ");
  myLCD->setCursor ( 8, 0 );
  myLCD->print ( tempFilter, 1 );
  myLCD->setCursor ( 12, 0 );
  myLCD->print ( "\x07" );
  myLCD->print ("C");
  drawBars ( tempFilter, 1, 14, '-', '+' );
  
  delay (LOOP_DELAY);
}