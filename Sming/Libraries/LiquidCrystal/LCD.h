// ---------------------------------------------------------------------------
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
// @file LCD.h
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK.
// 
// @brief 
// This is a basic implementation of the LiquidCrystal library of the
// Arduino SDK. This library is a refactored version of the one supplied
// in the Arduino SDK in such a way that it simplifies its extension
// to support other mechanism to communicate to LCDs such as I2C, Serial, SR, 
// The original library has been reworked in such a way that this will be
// the base class implementing all generic methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets.
//
// This base class is a pure abstract class and needs to be extended. As reference,
// it has been extended to drive 4 and 8 bit mode control, LCDs and I2C extension
// backpacks such as the I2CLCDextraIO using the PCF8574* I2C IO Expander ASIC.
//
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library.
//
// @version API 1.1.0
//
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#ifndef _LCD_H_
#define _LCD_H_

#if (ARDUINO <  100)
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include <inttypes.h>
#include <Print.h>


/*!
 @defined 
 @abstract   Enables disables fast waits for write operations for LCD
 @discussion If defined, the library will avoid doing un-necessary waits.
 this can be done, because the time taken by Arduino's slow digitalWrite
 operations. If fast digitalIO operations, comment this line out or undefine
 the mode.
 */
#ifdef __AVR__
#define FAST_MODE
#endif

/*!
 @function
 @abstract   waits for a given time in microseconds (compilation dependent).
 @discussion Waits for a given time defined in microseconds depending on
 the FAST_MODE define. If the FAST_MODE is defined the call will return
 inmediatelly.
 @param      uSec[in] time in microseconds.
 @result     None
 */
inline static void waitUsec ( uint16_t uSec )
{
#ifndef FAST_MODE
   delayMicroseconds ( uSec );
#endif // FAST_MODE
}


/*!
 @defined 
 @abstract   All these definitions shouldn't be used unless you are writing 
 a driver.
 @discussion All these definitions are for driver implementation only and
 shouldn't be used by applications.
 */
// LCD Commands
// ---------------------------------------------------------------------------
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

// flags for display entry mode
// ---------------------------------------------------------------------------
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off and cursor control
// ---------------------------------------------------------------------------
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

// flags for display/cursor shift
// ---------------------------------------------------------------------------
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

// flags for function set
// ---------------------------------------------------------------------------
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00


// Define COMMAND and DATA LCD Rs (used by send method).
// ---------------------------------------------------------------------------
#define COMMAND                 0
#define DATA                    1
#define FOUR_BITS               2


/*!
 @defined 
 @abstract   Defines the duration of the home and clear commands
 @discussion This constant defines the time it takes for the home and clear
 commands in the LCD - Time in microseconds.
 */
#define HOME_CLEAR_EXEC      2000

/*!
    @defined 
    @abstract   Backlight off constant declaration
    @discussion Used in combination with the setBacklight to swith off the
 LCD backlight. @set setBacklight
*/
#define BACKLIGHT_OFF           0

/*!
 @defined 
 @abstract   Backlight on constant declaration
 @discussion Used in combination with the setBacklight to swith on the
 LCD backlight. @set setBacklight
 */
#define BACKLIGHT_ON          255


/*!
 @typedef 
 @abstract   Define backlight control polarity
 @discussion Backlight control polarity. @see setBacklightPin.
 */
typedef enum { POSITIVE, NEGATIVE } t_backlighPol;

class LCD : public Print 
{
public:
   
   /*!
    @method
    @abstract   LiquidCrystal abstract constructor.
    @discussion LiquidCrystal class abstract constructor needed to create
    the base abstract class.
    */
   LCD ( );
   
   /*!
    @function
    @abstract   LCD initialization.
    @discussion Initializes the LCD to a given size (col, row). This methods
    initializes the LCD, therefore, it MUST be called prior to using any other
    method from this class.
    
    This method is abstract, a base implementation is available common to all LCD
    drivers. Should it not be compatible with some other LCD driver, a derived
    implementation should be done on the driver specif class.
    
    @param      cols[in] the number of columns that the display has
    @param      rows[in] the number of rows that the display has
    @param      charsize[in] character size, default==LCD_5x8DOTS
    */
   virtual void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
   
   /*!
    @function
    @abstract   Clears the LCD.
    @discussion Clears the LCD screen and positions the cursor in the upper-left 
    corner. 
    
    This operation is time consuming for the LCD.
    
    @param      none
    */
   void clear();
   
   /*!
    @function
    @abstract   Sets the cursor to the upper-left corner.
    @discussion Positions the cursor in the upper-left of the LCD. 
    That is, use that location in outputting subsequent text to the display. 
    To also clear the display, use the clear() function instead.
    
    This operation is time consuming for the LCD.
    
    @param      none
    */
   void home();
   
   /*!
    @function
    @abstract   Turns off the LCD display.
    @discussion Turns off the LCD display, without losing the text currently 
    being displayed on it.
    
    @param      none
    */
   void noDisplay();
   
   /*!
    @function
    @abstract   Turns on the LCD display.
    @discussion Turns on the LCD display, after it's been turned off with 
    noDisplay(). This will restore the text (and cursor location) that was on 
    the display prior to calling noDisplay().
    
    @param      none
    */
   void display();
   
   /*!
    @function
    @abstract   Turns off the blinking of the LCD cursor.
    
    @param      none
    */
   void noBlink();
   
   /*!
    @function
    @abstract   Display the cursor of the LCD.
    @discussion Display the blinking LCD cursor. If used in combination with 
    the cursor() function, the result will depend on the particular display.
    
    @param      none
    */
   void blink();
   
   /*!
    @function
    @abstract   Hides the LCD cursor.
    
    @param      none
    */
   void noCursor();
   
   /*!
    @function
    @abstract   Display the LCD cursor.
    @discussion Display the LCD cursor: an underscore (line) at the location 
    where the next character will be written.
    
    @param      none
    */
   void cursor();
   
   /*!
    @function
    @abstract   Scrolls the contents of the display (text and cursor) one space 
    to the left.
    
    @param      none
    */
   void scrollDisplayLeft();
   
   /*!
    @function
    @abstract   Scrolls the contents of the display (text and cursor) one space 
    to the right.
    
    @param      none
    */
   void scrollDisplayRight();
   
   /*!
    @function
    @abstract   Set the direction for text written to the LCD to left-to-right.
    @discussion Set the direction for text written to the LCD to left-to-right. 
    All subsequent characters written to the display will go from left to right, 
    but does not affect previously-output text.
    
    This is the default configuration.
    
    @param      none
    */
   void leftToRight();
   
   /*!
    @function
    @abstract   Set the direction for text written to the LCD to right-to-left.
    @discussion Set the direction for text written to the LCD to right-to-left. 
    All subsequent characters written to the display will go from right to left, 
    but does not affect previously-output text.
    
    left-to-right is the default configuration.
    
    @param      none
    */
   void rightToLeft();
   
   /*!
    @function
    @abstract   Moves the cursor one space to the left.
    @discussion 
    @param      none
    */   
   void moveCursorLeft();
   
   
   /*!
    @function
    @abstract   Moves the cursor one space to the right.
    
    @param      none
    */
   void moveCursorRight();
   
   /*!
    @function
    @abstract   Turns on automatic scrolling of the LCD.
    @discussion Turns on automatic scrolling of the LCD. This causes each 
    character output to the display to push previous characters over by one 
    space. If the current text direction is left-to-right (the default), 
    the display scrolls to the left; if the current direction is right-to-left, 
    the display scrolls to the right. 
    This has the effect of outputting each new character to the same location on 
    the LCD.
    
    @param      none
    */
   void autoscroll();
   
   /*!
    @function
    @abstract   Turns off automatic scrolling of the LCD.
    @discussion Turns off automatic scrolling of the LCD, this is the default
    configuration of the LCD.
    
    @param      none
    */
   void noAutoscroll();
   
   /*!
    @function
    @abstract   Creates a custom character for use on the LCD.
    @discussion Create a custom character (glyph) for use on the LCD. 
    Most chipsets only support up to eight characters of 5x8 pixels. Therefore,
    this methods has been limited to locations (numbered 0 to 7). 
    
    The appearance of each custom character is specified by an array of eight 
    bytes, one for each row. The five least significant bits of each byte 
    determine the pixels in that row. To display a custom character on screen, 
    write()/print() its number, i.e. lcd.print (char(x)); // Where x is 0..7.
    
    @param      location[in] LCD memory location of the character to create
    (0 to 7)
    @param      charmap[in] the bitmap array representing each row of the character.
    */
   void createChar(uint8_t location, uint8_t charmap[]);
   
   /*!
    @function
    @abstract   Position the LCD cursor.
    @discussion Sets the position of the LCD cursor. Set the location at which 
    subsequent text written to the LCD will be displayed.
    
    @param      col[in] LCD column
    @param      row[in] LCD row - line.
    */
   void setCursor(uint8_t col, uint8_t row);
   
   /*!
    @function
    @abstract   Switch-on the LCD backlight.
    @discussion Switch-on the LCD backlight.
    The setBacklightPin has to be called before setting the backlight for
    this method to work. @see setBacklightPin. 
    */
   void backlight ( void );
   
   /*!
    @function
    @abstract   Switch-off the LCD backlight.
    @discussion Switch-off the LCD backlight.
    The setBacklightPin has to be called before setting the backlight for
    this method to work. @see setBacklightPin. 
    */   
   void noBacklight ( void );
   
   /*!
    @function
    @abstract   Switch on the LCD module.
    @discussion Switch on the LCD module, it will switch on the LCD controller
    and the backlight. This method has the same effect of calling display and
    backlight. @see display, @see backlight
    */
   void on ( void );

   /*!
    @function
    @abstract   Switch off the LCD module.
    @discussion Switch off the LCD module, it will switch off the LCD controller
    and the backlight. This method has the same effect of calling noDisplay and
    noBacklight. @see display, @see backlight
    */   
   void off ( void );
   
   //
   // virtual class methods
   // --------------------------------------------------------------------------
   /*!
    @function
    @abstract   Sets the pin to control the backlight.
    @discussion Sets the pin in the device to control the backlight.
    This method is device dependent and can be programmed on each subclass. An 
    empty function call is provided that does nothing.
    
    @param      value: pin associated to backlight control.
    @param      pol: backlight polarity control (POSITIVE, NEGATIVE)
    */
   virtual void setBacklightPin ( uint8_t value, t_backlighPol pol ) { };
   
   /*!
    @function
    @abstract   Sets the pin to control the backlight.
    @discussion Sets the pin in the device to control the backlight. The behaviour
    of this method is very dependent on the device. Some controllers support
    dimming some don't. Please read the actual header file for each individual
    device. The setBacklightPin method has to be called before setting the backlight
    or the adequate backlight control constructor.
    @see setBacklightPin.
    
    NOTE: The prefered methods to control the backlight are "backlight" and
    "noBacklight".
    
    @param      0..255 - the value is very dependent on the LCD. However, 
    BACKLIGHT_OFF will be interpreted as off and BACKLIGHT_ON will drive the
    backlight on.
    */
   virtual void setBacklight ( uint8_t value ) { };
   
   /*!
    @function
    @abstract   Writes to the LCD.
    @discussion This method writes character to the LCD in the current cursor
    position.
    
    This is the virtual write method, implemented in the Print class, therefore
    all Print class methods will end up calling this method.
    
    @param      value[in] Value to write to the LCD.
    */
#if (ARDUINO <  100)
   virtual void write(uint8_t value);
#else
   virtual size_t write(uint8_t value);
#endif
   
#if (ARDUINO <  100)
   using Print::write;
#else
   using Print::write;
#endif   
   
protected:
   // Internal LCD variables to control the LCD shared between all derived
   // classes.
   uint8_t _displayfunction;  // LCD_5x10DOTS or LCD_5x8DOTS, LCD_4BITMODE or 
                              // LCD_8BITMODE, LCD_1LINE or LCD_2LINE
   uint8_t _displaycontrol;   // LCD base control command LCD on/off, blink, cursor
                              // all commands are "ored" to its contents.
   uint8_t _displaymode;      // Text entry mode to the LCD
   uint8_t _numlines;         // Number of lines of the LCD, initialized with begin()
   uint8_t _cols;             // Number of columns in the LCD
   t_backlighPol _polarity;   // Backlight polarity
   
private:
   /*!
    @function
    @abstract   Send a command to the LCD.
    @discussion This method sends a command to the LCD by setting the Register
    select line of the LCD.
    
    This command shouldn't be used to drive the LCD, only to implement any other
    feature that is not available on this library.
    
    @param      value[in] Command value to send to the LCD (COMMAND, DATA or
    FOUR_BITS).
    */
   void command(uint8_t value);

   /*!
    @function
    @abstract   Send a particular value to the LCD.
    @discussion Sends a particular value to the LCD. This is a pure abstract
    method, therefore, it is implementation dependent of each derived class how
    to physically write to the LCD.
    
    Users should never call this method.
    
    @param      value[in] Value to send to the LCD.
    @result     mode LOW - write to the LCD CGRAM, HIGH - write a command to
    the LCD.
    */
#if (ARDUINO <  100)
   virtual void send(uint8_t value, uint8_t mode) { };
#else
   virtual void send(uint8_t value, uint8_t mode) = 0;
#endif
   
};

#endif
