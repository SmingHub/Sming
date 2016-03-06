// ---------------------------------------------------------------------------
// Created/Adapted by Bill Perry 2012-03-16
// Copyright 2012 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
//
// @file LiquidCrystal_SR2W.h
// Connects a hd44780 LCD using 2 pins from the Arduino, via an 8-bit
// ShiftRegister (SR2W from now on).
//
// @brief
// This is the 2 wire shift register interface class for the LCD library
//
// The functionality provided by this class and its base class is a superset of
// the original functionality of the Arduino LiquidCrystal library and can
// be used as such.
// See the LCD class for a full description of the API functions available.
//
// It works with a 8-bit unlatched, no-tristate, unidirectional SIPO (Serial-In-Parallel-Out)
// shift register (IE a very simple SR), and an hd44780 LCD in 4-bit mode.
// Any such shift register should do (pref. 74LS family IC's for 2-wire).
// 74LS164 and 74HC595 have been exstensively tested.
//
//
// 2 Pins required from the Arduino:
// - Data/Enable
// - Clock
// The Data pin is also used to control the enable signal
// LCD RW-pin hardwired to LOW (only writing to LCD).
// Busy Flag (BF, data bit D7) is not read.
//
//  Original project homepage: http://code.google.com/p/arduinoshiftreglcd/
//
// Shift register bits
// Bit  #0   - (cannot be used on 74HC595)
// Bit  #1   - optional backlight control
// Bit  #2   - connects to RS (Register Select) on the LCD
// Bit  #3   - connects to LCD data inputs D4
// Bit  #4   - connects to LCD data inputs D5
// Bit  #5   - connects to LCD data inputs D6
// Bit  #6   - connects to LCD data inputs D7
// Bit  #7   - enables the LCD enable-puls (via the diode-resistor AND "gate")
//
// Wiring for a 74LS164
// ---------------------
//                          1k/4.7k
//              +--------[ Resistor ]--------+---(LCD Enable)
//              |                            |
//              |          74LS164    (VCC)  |
//              |        +----u----+    |   _V_ diode
// (data pin)---+---+--1-|A     VCC|-14-+    |
//                  |    |         |         |
//                  +--2-|B      Q7|-13------+
//                     3-|Q0     Q6|-12--(LCD D7)
// (BL Circuit)--------4-|Q1     Q5|-11--(LCD D6)
// (LCD RS)------------5-|Q2     Q4|-10--(LCD D5)
// (LCD D4)------------6-|Q3    /MR|--9--(VCC)
//                   +-7-|GND    CP|--8--(clock pin)
//                   |   +---------+
//                   |      0.1uf
//                 (gnd)-----||----(vcc)
//
// Wiring for a 74HC595
// --------------------
// NOTE: the 74HC595 is a latching shift register. In order to get it to operate
// in a "non latching" mode, RCLK and SCLK are tied together. The side effect of this
// is that the latched output is one clock behind behind the internal shift register bits.
// To compensate for this the wiring is offset by one bit position lower.
// For example, while the backlight is hooked to Q0 it is still using bit 1 of
// of the shift register because the outputs are 1 clock behind the real internal shift
// register.
//
//                         74HC595    (VCC)
//                       +----u----+    |  +-----------------------(BL circuit)
// (LCD RS)------------1-|Q1    VCC|-16-+  |  +--------------------(data pin)
// (LCD D4)------------2-|Q2     Q0|-15----+  |      1k/4.7k
// (LCD D5)------------3-|Q3    SER|-14-------+---[ Resistor ]--+--(LCD Enable)
// (LCD D6)------------4-|Q4    /OE|-13--(gnd)                  |
// (LCD D7)------------5-|Q5   RCLK|-12-------+                 |
//                       |         |          |                 |
//              +------6-|Q6   SCLK|-11-------+--(clock pin)    |
//              |      7-|Q7    /MR|-10--(VCC)                  |
//              |    +-8-|GND   Q6'|--9                         |
//              |    |   +---------+                    diode  _V_
//              |    |      0.1uf                               |
//              |  (gnd)-----||----(vcc)                        |
//              +-----------------------------------------------+
//
//
// Backlight Control circuit
// -------------------------
// Because the shift resiter is not latching the outputs, the backlight circuitry
// will "see" the output bits as they are shifted into the shift register which
// can cause the backlight to flicker rather than remain constantly on/off.
// The circuit below slows down the transitions to the transistor to remove
// the visible flicker. When the BL input is HIGH the LCD backlight will turn on.
//
//                (value depends on LCD, 100ohm is usually safe)
// (LCD BL anode)---[ resistor ]---(vcc)
//
// (LCD BL cathode)-------------------------------+
//                                                |
//                                                D
//                                                |
// (BL input)----[ 4.7k Resistor ]----+-------G-|-<  (2N7000 FET)
//                                    |           |
//                          (0.1uf)   =           S
//                                    |           |
//                                  (gnd)        (gnd)
//
//
//
//
// History
// 2012.03.16  bperrybap - creation/adaption from SR header to create SR2W header.
//                         Fixed typo in SR2W mask define names
// @author B. Perry - bperrybap@opensource.billsworld.billandterrie.com
// --------------------------------------------------------------------------------
#ifndef _LIQUIDCRYSTAL_SR2W_
#define _LIQUIDCRYSTAL_SR2W_

#include <inttypes.h>

#include "FastIO.h"
#include "LCD.h"


// two-wire SR output bit constants
// ---------------------------------------------------------------------------

#define SR2W_BL_MASK 0x02
#define SR2W_RS_MASK 0x04
#define SR2W_DATA_MASK 0x78	// data bits are hard coded to be SR bits 6,5,4,3
#define SR2W_EN_MASK 0x80	// cannot ever be changed

class LiquidCrystal_SR2W : public LCD
{
public:
   /*!
    @method
    @abstract   LCD 2 wire SHIFT REGISTER constructor.
    @discussion Defines the pin assignments that connect to the shift register.
    The constructor does not initialize the LCD. Assuming 1 line 8 pixel high
    font.

    @param srdata[in]   Arduino pin for shift register data line.
    @param srclock[in]  Arduino pin for shift register clock line.
    @param blpol[in]    optional backlight polarity (default = POSITIVE)
    */
   LiquidCrystal_SR2W (uint8_t srdata, uint8_t srclock, t_backlighPol blpol = POSITIVE);

   /*!
    @function
    @abstract   Send a particular value to the LCD.
    @discussion Sends a particular value to the LCD for writing to the LCD or
    as an LCD command using the shift register.

    Users should never call this method.

    @param      value[in] Value to send to the LCD.
    @param      mode[in]  DATA=8bit data, COMMAND=8bit cmd, FOUR_BITS=4bit cmd
    the LCD.
    */
   virtual void send(uint8_t value, uint8_t mode);


   /*!
    @function
    @abstract   Switch-on/off the LCD backlight.
    @discussion Switch-on/off the LCD backlight.
    The setBacklightPin has to be called before setting the backlight for
    this method to work. @see setBacklightPin.

    @param      mode[in] backlight mode (0 off, non-zero on)
    */
   void setBacklight ( uint8_t mode );

private:

   /*!
    @method
    @abstract   Initializes the LCD pin allocation
    @discussion Initializes the LCD pin allocation and configuration.
    */
   void init ( uint8_t srdata, uint8_t srclock, t_backlighPol blpol, uint8_t lines, uint8_t font );

   /*!
    * @method
    * @abstract takes care of shifting and the enable pulse
    */
   void loadSR (uint8_t val);

   fio_register _srDataRegister; // Serial Data pin
   fio_bit _srDataMask;
   fio_register _srClockRegister; // Clock Pin
   fio_bit _srClockMask;

   uint8_t _blPolarity;
   uint8_t _blMask;
};
#endif
