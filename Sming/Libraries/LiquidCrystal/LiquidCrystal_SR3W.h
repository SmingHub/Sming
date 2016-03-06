// ---------------------------------------------------------------------------
// Created by Francisco Malpartida on 7.3.2012.
// Copyright 2011 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file LiquidCrystal_SR3W.h
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK but using a generic SHIFT REGISTER extension board.
//
// @brief
// This is a basic implementation of the LiquidCrystal library of the
// Arduino SDK. The original library has been reworked in such a way that
// this class implements the all methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets using a 3 wire latching
// shift register. While it has been tested with a 74HC595N shift register
// it should also work with other latching shift registers such as the MC14094
// and the HEF4094
//
// This particular driver has been created as generic as possible to enable
// users to configure and connect their LCDs using just 3 digital IOs from the
// AVR or Arduino, and connect the LCD to the outputs of the shiftregister
// in any configuration. The library is configured by passing the IO pins
// that control the strobe, data and clock of the shift register and a map
// of how the shiftregister is connected to the LCD.
//
//
//   +--------------------------------------------+
//   |                 MCU                        |
//   |   IO1           IO2           IO3          |
//   +----+-------------+-------------+-----------+
//        |             |             |
//        |             |             |
//   +----+-------------+-------------+-----------+
//   |    Strobe        Data          Clock       |
//   |          8-bit shift/latch register        | 74HC595N
//   |    Qa0  Qb1  Qc2  Qd3  Qe4  Qf5  Qg6  Qh7  |
//   +----+----+----+----+----+----+----+----+----+
//        |    |    |    |    |    |    |
//        |11  |12  |13  |14  |6   |5   |4   (LCD pins)
//   +----+----+----+----+----+----+----+----+----+
//   |    DB4  DB5  DB6  DB7  E    Rw   RS        |
//   |                 LCD Module                 |
//
// NOTE: Rw is not used by the driver so it can be connected to GND.
//
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library.
//
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#ifndef _LIQUIDCRYSTAL_SR3W_H_
#define _LIQUIDCRYSTAL_SR3W_H_

#include <inttypes.h>

#include "FastIO.h"
#include "LCD.h"


class LiquidCrystal_SR3W : public LCD
{
public:

   /*!
    @method
    @abstract   Class constructor.
    @discussion Initializes class variables and defines the IO driving the
    shift register. The constructor does not initialize the LCD.
    Default configuration:
       Shift register      LCD
       QA - 0              DB4
       QB - 1              DB5
       QC - 2              DB6
       QD - 3              DB7
       QE - 4              E
       QF - 5
       QG - 6              Rs
       GND                 Rw

    @param      strobe[in] digital IO connected to shiftregister strobe pin.
    @param      data[in] digital IO connected to the shiftregister data pin.
    @param      clk[in] digital IO connected to the shiftregister clock pin.
    */
   LiquidCrystal_SR3W(uint8_t data, uint8_t clk, uint8_t strobe);
   // Constructor with backlight control
   LiquidCrystal_SR3W(uint8_t data, uint8_t clk, uint8_t strobe,
                      uint8_t backlighPin, t_backlighPol pol);

   /*!
    @method
    @abstract   Class constructor.
    @discussion Initializes class variables and defines the control lines of
    the LCD and the shiftregister. The constructor does not initialize the LCD.

    @param      strobe[in] digital IO connected to shiftregister strobe pin.
    @param      data[in] digital IO connected to shiftregister data pin.
    @param      clk[in] digital IO connected to shiftregister clock pin.
    @param      En[in] LCD En (Enable) pin connected to SR output pin.
    @param      Rw[in] LCD Rw (Read/write) pin connected to SR output pin.
    @param      Rs[in] LCD Rs (Reg Select) pin connected to SR output pin.
    @param      d4[in] LCD data 4 pin map to the SR output pin.
    @param      d5[in] LCD data 5 pin map to the SR output pin.
    @param      d6[in] LCD data 6 pin map to the SR output pin.
    @param      d7[in] LCD data 7 pin map to the SR output pin.
    */
   LiquidCrystal_SR3W(uint8_t data, uint8_t clk, uint8_t strobe,
                      uint8_t En, uint8_t Rw, uint8_t Rs,
                      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7 );
   // Constructor with backlight control
   LiquidCrystal_SR3W( uint8_t data, uint8_t clk, uint8_t strobe,
                      uint8_t En, uint8_t Rw, uint8_t Rs,
                      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                      uint8_t backlighPin, t_backlighPol pol);

   /*!
    @function
    @abstract   Send a particular value to the LCD.
    @discussion Sends a particular value to the LCD for writing to the LCD or
    as an LCD command.

    Users should never call this method.

    @param      value[in] Value to send to the LCD.
    @param      mode[in] DATA - write to the LCD CGRAM, COMMAND - write a
    command to the LCD.
    */
   virtual void send(uint8_t value, uint8_t mode);

   /*!
    @function
    @abstract   Sets the pin to control the backlight.
    @discussion Sets the pin in the device to control the backlight. This device
    doesn't support dimming backlight capability.

    @param      0: backlight off, 1..255: backlight on.
    */
   void setBacklightPin ( uint8_t value, t_backlighPol pol );

   /*!
    @function
    @abstract   Switch-on/off the LCD backlight.
    @discussion Switch-on/off the LCD backlight.
    The setBacklightPin has to be called before setting the backlight for
    this method to work. @see setBacklightPin.

    @param      value: backlight mode (HIGH|LOW)
    */
   void setBacklight ( uint8_t value );

private:

   /*!
    @method
    @abstract   Initializes the LCD class
    @discussion Initializes the LCD class and IO expansion module.
    */
   int  init(uint8_t data, uint8_t clk, uint8_t strobe,
             uint8_t Rs, uint8_t Rw, uint8_t En,
             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

   /*!
    @method
    @abstract   Writes an 4 bit value to the LCD.
    @discussion Writes 4 bits (the least significant) to the LCD control data lines.
    @param      value[in] Value to write to the LCD
    @param      more[in]  Value to distinguish between command and data.
    COMMAND == command, DATA == data.
    */
   void write4bits(uint8_t value, uint8_t mode);

   /*!
    @function
    @abstract   load into the shift register a byte
    @discussion loads into the shift register a byte
    @param      value[in]: value to be loaded into the shiftregister.
    */
   void loadSR(uint8_t value);


   fio_bit      _strobe;           // shift register strobe pin
   fio_register _strobe_reg;       // SR strobe pin MCU register
   fio_bit      _data;             // shift register data pin
   fio_register _data_reg;         // SR data pin MCU register
   fio_bit      _clk;              // shift register clock pin
   fio_register _clk_reg;          // SR clock pin MCU register
   uint8_t      _En;               // LCD expander word for enable pin
   uint8_t      _Rw;               // LCD expander word for R/W pin
   uint8_t      _Rs;               // LCD expander word for Register Select pin
   uint8_t      _data_pins[4];     // LCD data lines
   uint8_t      _backlightPinMask; // Backlight IO pin mask
   uint8_t      _backlightStsMask; // Backlight status mask

};

#endif

