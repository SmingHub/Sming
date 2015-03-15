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
// @file LiquidCrystal_SR.h
//  Connects an LCD using 2 or 3 pins from the Arduino, via an 8-bit 
// ShiftRegister (SR from now on).
// 
// @brief 
// This is a port of the ShiftRegLCD library from raron and ported to the
// LCD library.
//
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library and can
// be used as such.
//
// Modified to work serially with the shiftOut() function, an 8-bit
// unlatched, no-tristate, unidirectional SIPO (Serial-In-Parallel-Out)
// shift register (IE a very simple SR), and an LCD in 4-bit mode.
// Any such shift register should do (pref. 74LS family IC's for 2-wire).
// I used 74LS164, for the reason that's what I had at hand.
//
// Connection description:
//
// SR output:
// Bit  #0   - N/C - not connected, used to hold a zero
// Bit  #1   - N/C
// Bit  #2   - connects to RS (Register Select) on the LCD
// Bits #3-6 - connects to LCD data inputs D4 - D7.
// Bit  #7   - enables the LCD enable-puls (via the diode-resistor AND "gate")
//
// 2 or 3 Pins required from the Arduino for Data, Clock and (optional) Enable
// If not using Enable, the Data pin will be used for the enable signal.
// 2 wire mode can be indicated by:
// - ommitting the enable pin in constructor
// - defining the same pin for Enable as for Data in constructor
// - by using the token TWO_WIRE for the enable pin.
//
// Data and Clock outputs/pins goes to the shiftregister.
// LCD RW-pin hardwired to LOW (only writing to LCD).
// Busy Flag (BF, data bit D7) is not read.
//
//  Original project homepage: http://code.google.com/p/arduinoshiftreglcd/
//
//
// History
// 2012.03.29  bperrybap - can now eliminate enable pin in constructor for two wire mode.
// 2011.10.29  fmalpartida - adaption of the library to the LCD class hierarchy.
// 2011.07.02  Fixed a minor flaw in setCursor function. No functional change, 
//             just a bit more memory efficient.
//             Thanks to CapnBry (from google code and github) who noticed it.
//             URL to his version of shiftregLCD:
//             https://github.com/CapnBry/HeaterMeter/commit/c6beba1b46b092ab0b33bcbd0a30a201fd1f28c1
// 2009.07.30  raron - minor corrections to the comments.
//             Fixed timing to datasheet safe. Fixed keyword highlights.
// 2009.07.28  Mircho / raron - a new modification to the schematics, and a
//             more streamlined interface
// 2009.07.27  Thanks to an excellent suggestion from mircho at the Arduiono 
//             playgrond forum, the number of wires now required is only two!
// 2009.07.25  raron - Fixed comments. I really messed up the comments before 
//             posting this, so I had to fix it.
//             Renamed a function, but no improvements or functional changes.
// 2009.07.23  Incorporated some proper initialization routines
//             inspired (lets say copy-paste-tweaked) from LiquidCrystal
//             library improvements from LadyAda.
// 2009.05.23  raron - first version, but based mostly (as in almost verbatim)
//             on the "official" LiquidCrystal library.
//
//
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#ifndef _LIQUIDCRYSTAL_SR_
#define _LIQUIDCRYSTAL_SR_

#include <inttypes.h>

#include "FastIO.h"
#include "LCD.h"


// two-wire indicator constant
// ---------------------------------------------------------------------------
#define TWO_WIRE  204
#define SR_RS_BIT 0x04
#define SR_EN_BIT 0x80

class LiquidCrystal_SR : public LCD
{
public:
   /*!
    @method     
    @abstract   LCD SHIFT REGISTER constructors.
    @discussion Defines the pin assignment that the LCD will have.
    The constructor does not initialize the LCD. Assuming 1 line 8 pixel high 
    font.
    
    @param srdata[in]   pin for shiftregister data line.
    @param srclock[in]  pin for shiftregister clock line.
    @param enable[in]   optional direct enable pin for the LCD
    */
   LiquidCrystal_SR ( uint8_t srdata, uint8_t srclock, uint8_t enable=TWO_WIRE );
   
   /*!
    @function
    @abstract   Send a particular value to the LCD.
    @discussion Sends a particular value to the LCD for writing to the LCD or
    as an LCD command using the shift register.
    
    Users should never call this method.
    
    @param      value[in] Value to send to the LCD.
    @result     mode LOW - write to the LCD CGRAM, HIGH - write a command to
    the LCD.
    */
   virtual void send(uint8_t value, uint8_t mode);
   
   
   /*!
    @function
    @abstract   Sets the pin to control the backlight.
    @discussion Sets the pin in the device to control the backlight.
    @warning    Currently not supported
    
    @param      mode: backlight mode (HIGH|LOW)
    @param      pol: backlight polarity
    */
   void setBacklightPin ( uint8_t pin, t_backlighPol pol );
   
   /*!
    @function
    @abstract   Switch-on/off the LCD backlight.
    @discussion Switch-on/off the LCD backlight.
    The setBacklightPin has to be called before setting the backlight for
    this method to work. @see setBacklightPin.
    
    @param      mode: backlight mode (HIGH|LOW)
    */
   void setBacklight ( uint8_t mode );
   
private:
   
   /*!
    @method     
    @abstract   Initializes the LCD pin allocation
    @discussion Initializes the LCD pin allocation and configuration.
    */
   void init ( uint8_t srdata, uint8_t srclock, uint8_t enable, uint8_t lines, 
              uint8_t font );
   
   /*!
    * @method
    * @abstract takes care of shifting and the enable pulse
    */
   void shiftIt (uint8_t val);
   
   uint8_t _enable_pin;  // Enable Pin
   uint8_t _two_wire;    // two wire mode
   
   fio_register _srDataRegister; // Serial Data pin
   fio_bit _srDataBit;
   fio_register _srClockRegister; // Clock Pin
   fio_bit _srClockBit;
   fio_register _srEnableRegister; // Enable Pin
   fio_bit _srEnableBit;
   
};

#endif

