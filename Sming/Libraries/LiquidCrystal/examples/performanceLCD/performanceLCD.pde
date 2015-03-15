// Created by Francisco Malpartida on 20/08/11.
// Copyright 2011 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file performanceLCD.h
// This sketch implements a simple benchmark for the New LiquidCrystal library.
// 
// @brief 
// This sketch provides a simple benchmark for the New LiquidCrystal library. It
// enables to test the varios classes provided by the library giving a performance
// reference.
//
// This library is only compatible with Arduino's SDK version 1.0
//
// @version API 1.0.0
//
// @author F. Malpartida - fmalpartida@gmail.com
//         Contribution by flo - Florian@Fida.biz - for benchmarking SR 
// ---------------------------------------------------------------------------
#include <Wire.h>

#define _LCD_I2C_

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

#ifdef _LCD_SR1_
#include <LiquidCrystal_SR1.h>
#endif

// C runtime variables
// -------------------
#ifdef __AVR__
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;
#endif

// Constants and definitions
// -------------------------
// Definitions for compatibility with Arduino SDK prior to version 1.0
#ifndef F
#define F(str) str
#endif

/*!
 @defined    NUM_BENCHMARKS
 @abstract   Number of benchmarks in the project.
 */
#define NUM_BENCHMARKS 4

/*!
 @defined    ITERATIONS
 @abstract   Number of benchmarks iterations to perform.
 */
#define ITERATIONS    10

/*!
 @defined    LCD_ROWS
 @abstract   LCD rows
 @discussion Defines the number of rows that the LCD has, normal LCD ranges are (1, 2, 4).
 */
#define LCD_ROWS        2

/*!
 @defined    LCD_COLUMNS
 @abstract   LCD available columns
 @discussion Defines the number of colums that the LCD has, normal LCD ranges are (8, 16, 20).
 */
#define LCD_COLUMNS    16

/*!
 @const      Pin constant definitions
 @abstract   Define several constants required to manage the LCD backlight and contrast
 */
#ifdef _LCD_I2C_
const int   BACKLIGHT_PIN  = 12;
const int   CONTRAST_PIN  = 0; // none
const int   CONTRAST      = 0; // none
#endif

#ifdef _LCD_4BIT_
const int    CONTRAST_PIN  = 9;
const int    BACKLIGHT_PIN = 7;
const int    CONTRAST      = 120;
#endif

#ifdef _LCD_SR_
const int    CONTRAST_PIN  = 0; // not connected
const int    BACKLIGHT_PIN = 0; // none
const int    CONTRAST      = 0;
#endif

#ifdef _LCD_SR1_
const int    CONTRAST_PIN  = 0; // not connected
const int    BACKLIGHT_PIN = 0; // none
const int    CONTRAST      = 0;
#endif

#ifdef _LCD_SR3W_
const int    CONTRAST_PIN  = 0; // none
const int    BACKLIGHT_PIN = 5;
const int    CONTRAST      = 0;
#endif

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
   { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0 }
};

/*!
 @typedef    t_benchmarkOp
 @abstract   Function pointer associated to each benchmark.
 */
typedef long (*t_benchmarkOp)( uint8_t );

/*!
 @typedef    t_timeBenchMarks
 @abstract   Structure to store results of the execution time of the benchmark.
 @field      benchmark: function pointer of the benchmark to be executed.
 */
typedef struct 
{
   t_benchmarkOp   benchmark; /**< Function pointer associated to the benchmark */
   long            benchTime; /**< execution time for benchmark 1 in useconds */
   uint16_t        numWrites; /**< Number of write cycles of the benchmark    */
} t_benchMarks;


// Main LCD objects
// ----------------
#ifdef _LCD_I2C_
LiquidCrystal_I2C lcd(0x38);  // set the LCD address to 0x20 for a 16 chars and 2 line display
#endif

#ifdef _LCD_4BIT_
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#endif

#ifdef _LCD_SR_
LiquidCrystal_SR lcd(8,7,TWO_WIRE);
#endif

#ifdef _LCD_SR1_
LiquidCrystal_SR1 lcd(2);
#endif

#ifdef _LCD_SR3W_
LiquidCrystal_SR3W lcd(3, 2, 4);
#endif

// benchMarks definitions
// ----------------------
extern long benchmark1 ( uint8_t );
extern long benchmark2 ( uint8_t );
extern long benchmark3 ( uint8_t );
extern long benchmark4 ( uint8_t );

//! @brief benchmark structure that will be initialised and 
static t_benchMarks myBenchMarks[NUM_BENCHMARKS] =
{
   { benchmark1, 0, (LCD_ROWS * LCD_COLUMNS) + 2 },
   { benchmark2, 0, LCD_ROWS * LCD_COLUMNS * 6 * 2 },
   { benchmark3, 0, 40 + 2 },
   { benchmark4, 0, 40 + 2 }
};

// Static methods
// --------------
/*!
 @function   freeMemory
 @abstract   Return available RAM memory
 @discussion This routine returns the ammount of RAM memory available after
 initialising the C runtime.
 @param      
 @return     Free RAM available, -1 for non AVR microcontrollers
 */
static int freeMemory ( void ) 
{
#ifdef __AVR__
   int free_memory;
   
   if((int)__brkval == 0)
   free_memory = ((int)&free_memory) - ((int)&__bss_end);
   else
   free_memory = ((int)&free_memory) - ((int)__brkval);
   
   return free_memory;
#else
   return -1;
#endif

}

/*!
 @function   LCDSetup
 @abstract   Initialise LCD associated pins and initialise the LCD object 
 with its geometry.
 @discussion Initialise the LCD object and make it ready for operation by
 setting up the LCD geometry, i.e. LCD character size. Initialise
 and configure all associated control pins such as backlight and
 contras pin if necessary.
 
 @param[in]  charBitmapSize: contrasts pin associated to the contrast (should be an
 analog pin). 0 if contrast pin is not required.
 @param[in]  backlight: backlight pin associated to the LCD backlight.
 @param[in]  cols: number of LCD columns normal values (1, 2, 4)
 @param[in]  rows: number of LCD rows normal values (8, 16, 20)
 */
static void LCDSetup ( uint8_t contrasPin, uint8_t backlight, uint8_t cols, uint8_t rows )
{
   // If our setup uses a PWM to control the backlight, configure it
   // --------------------------------------------------------------
   if ( contrasPin != 0 )
   {
      pinMode ( contrasPin, OUTPUT );
      analogWrite ( contrasPin, CONTRAST );
   }
   // Setup backlight pin
   if ( backlight != 0 ){
     pinMode(backlight, OUTPUT);
     digitalWrite(backlight, HIGH);
   }
   
   lcd.begin ( cols, rows );
   lcd.clear ( );
}


/*!
 @function   LCDLoadCharacters
 @abstract   Loads onto the LCD the character set for the benchmark.
 @discussion Loads onto the LCD the character set that will be used throughout
 the benchmark.
 
 @param[in]  charBitmapSize: number of characters to load to the LCD.
 */
static void LCDLoadCharacters ( int numChars )
{
   // Load custom character set into CGRAM
   for ( int i = 0; i < numChars; i++ )
   {
      lcd.createChar ( i, (uint8_t *)charBitmap[i] );
   }
}


// Benchmarks
// ----------
/*!
 @function   benchmark1
 @abstract   writes to the LCD a full set of characters loaded on the LCD
             memory.
 @discussion Writes to all the positions of the LCD a fixed pattern from
             memory. For every line it writes, it positions the cursor.
             The number of writen LCD accesses is: LCD_ROW * LCD_COLUMS + 2.
             It returns the cumulative time used by all the iterations.
 
 @param[in]  iterations: number of iterations the benchmark is executed before
             returning the time taken by all iterations.
 @return     The time take to execute iterations number of benchmarks.
 */
long benchmark1 ( uint8_t iterations )
{
   unsigned long time, totalTime = 0;
   int i, j;
   
   while ( iterations > 0 )
   {
      // Clear the LCD
      lcd.clear ( );
   
      time = micros ();
      for ( i = 0; i < LCD_ROWS; i++ )
      {
         lcd.setCursor ( 0, i );
         for ( j = 0; j < LCD_COLUMNS; j++ )
         {
            lcd.print (char(5));
         }
      }
      totalTime += ( micros() - time );
      delay ( 200 ); // it doesn't keep up with the LCD refresh rate.
      iterations--;
   }
   return ( totalTime );
}

/*!
 @function   benchmark2
 @abstract   writes to the LCD a full set of characters loaded on the LCD
             memory one line pixel at the time
 @discussion Writes to all the positions of the LCD a fixed pattern from
             memory each patern take 6 write operations to the LCD. For every  
             character it writes it sets the cursor possition.
             The number of writen LCD accesses is: LCD_ROW * LCD_COLUMS * 6.
             It returns the cumulative time used by all the iterations.
 
 @param[in]  iterations: number of iterations the benchmark is executed before
             returning the time taken by all iterations.
 @return     The time take to execute iterations number of benchmarks.
 */
long benchmark2 ( uint8_t iterations )
{
   unsigned long time, totalTime = 0;
   int i, j, k;
   
   while ( iterations > 0 )
   {
      // Clear the LCD
      lcd.clear ( );
   
      time = micros ();
      
      for ( i = 0; i < LCD_ROWS; i++ )
      {
         for ( j = 0; j < LCD_COLUMNS; j++ )
         {
            for ( k = 0; k <= 5; k++ )
            {
               lcd.setCursor ( j, i );
               lcd.print (char(k));
            }
         }
      }
      totalTime += ( micros() - time );
      iterations--;
   }
   return ( totalTime );
}

/*!
 @function   benchmark3
 @abstract   writes to the LCD a full set of characters from memory.
 @discussion Writes to all the positions of the LCD a fixed pattern from
             RAM. For every line it writes, it positions the cursor.
             The number of writen LCD accesses is: LCD_ROW * LCD_COLUMS + 2.
             It returns the cumulative time used by all the iterations.
 
 @param[in]  iterations: number of iterations the benchmark is executed before
             returning the time taken by all iterations.
 @return     The time take to execute iterations number of benchmarks.
 */
long benchmark3 ( uint8_t iterations )
{
   unsigned long time, totalTime = 0;
   int i;
   
   while ( iterations > 0 )
   {
      // Clear the LCD
      lcd.clear ( );
   
      time = micros ();
      for ( i = 0; i < LCD_ROWS; i++ )
      {
         lcd.setCursor ( 0, i );
         lcd.print ( "####################" );
      }
      totalTime += ( micros() - time );
      delay ( 200 ); // it doesn't keep up with the LCD refresh rate.
      iterations--;
   }
   return ( totalTime );
}

/*!
 @function   benchmark4
 @abstract   writes to the LCD a full set of characters from memory.
 @discussion Writes to all the positions of the LCD a fixed pattern from
             flash. For every line it writes, it positions the cursor.
             The number of writen LCD accesses is: LCD_ROW * LCD_COLUMS + 2.
             It returns the cumulative time used by all the iterations.
 
 @param[in]  iterations: number of iterations the benchmark is executed before
             returning the time taken by all iterations.
 @return     The time take to execute iterations number of benchmarks.
 */
long benchmark4 ( uint8_t iterations )
{
   unsigned long time, totalTime = 0;
   int i;
   
   while ( iterations > 0 )
   {
      // Clear the LCD
      lcd.clear ( );
   
      time = micros ();
      for ( i = 0; i < LCD_ROWS; i++ )
      {
         lcd.setCursor ( 0, i );
         lcd.print ( F("####################") );
      }
      totalTime += ( micros() - time );
      delay ( 200 ); // it doesn't keep up with the LCD refresh rate.
      iterations--;
   }
   return ( totalTime );
}

// Main system setup
// -----------------
void setup ()
{
   Serial.begin ( 57600 );
#ifdef __AVR__
   Serial.print ( F("Free mem: ") );
   Serial.println ( freeMemory () );
#endif
   
   // Initialise the LCD
   LCDSetup ( CONTRAST_PIN, BACKLIGHT_PIN, LCD_COLUMNS, LCD_ROWS );
   LCDLoadCharacters ( (sizeof(charBitmap ) / sizeof (charBitmap[0])) );
}


// Main system loop
// ----------------
void loop ()
{
   int i;

   lcd.setCursor ( 0, 0 );
   lcd.clear ( );
   
   // Run benchmark
   for ( i = 0; i < NUM_BENCHMARKS; i++ )
   {
      myBenchMarks[i].benchTime = 
         (myBenchMarks[i].benchmark (ITERATIONS))/ITERATIONS;
         Serial.println (i);
   }
   
   float fAllWrites=0.0;
   
   for ( i = 0; i < NUM_BENCHMARKS; i++ )
   {   
      Serial.print ( F("benchmark") );
      Serial.print ( i );
      Serial.print ( F(": ") );
      Serial.print ( myBenchMarks[i].benchTime );
      Serial.print ( F(" us - ") );
      Serial.print ( F(" write: ") );
      Serial.print ( myBenchMarks[i].benchTime / (float)myBenchMarks[i].numWrites );
      Serial.println ( F(" us") );
      fAllWrites += myBenchMarks[i].benchTime / (float)myBenchMarks[i].numWrites;
   }
   Serial.print( F("avg. write: ") );
   Serial.println( fAllWrites / (float)NUM_BENCHMARKS );
 }