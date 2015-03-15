// ---------------------------------------------------------------------------
// Created/Adapted by Bill Perry 2012-03-16
// Copyright 2012 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file LiquidCrystal_SR2W.cpp
// Connects a hd44780 LCD using 2 pins from the Arduino, via an 8-bit 
// ShiftRegister (SR2W from now on).
// 
// @brief 
// This is a port of the ShiftRegLCD library from raron and ported to the
// LCD library.
//
//
// See the corresponding SR2W header file for full details.
//
// History
// 2012.03.29  bperrybap - Fixed incorrect use of 5x10 for default font 
//                         (now matches original LQ library)
//                         Fixed typo in SR2W mask define names
//                         changed default backlight state to on
// 2012.03.16  bperrybap - created/modified from SR sources to create SR2W
// @author B. Perry - bperrybap@opensource.billsworld.billandterrie.com
// ---------------------------------------------------------------------------

#include "LiquidCrystal_SR2W.h"

// CONSTRUCTORS
// ---------------------------------------------------------------------------
// Assuming 1 line 8 pixel high font
LiquidCrystal_SR2W::LiquidCrystal_SR2W (uint8_t srdata, uint8_t srclock, t_backlighPol blpol)
{
	init ( srdata, srclock, blpol, 1, 0 );
}


// PRIVATE METHODS
// ---------------------------------------------------------------------------

//
// init
void LiquidCrystal_SR2W::init(uint8_t srdata, uint8_t srclock, t_backlighPol blpol, uint8_t lines, uint8_t font)
{
	_srDataRegister = fio_pinToOutputRegister(srdata);
	_srDataMask = fio_pinToBit(srdata);
	_srClockRegister = fio_pinToOutputRegister(srclock);
	_srClockMask = fio_pinToBit(srclock);
   
	_blPolarity = blpol;
   
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
   
	backlight(); // set default backlight state to on
}

//
// loadSR
void LiquidCrystal_SR2W::loadSR(uint8_t val)
{
	// Clear to keep Enable LOW while clocking in new bits
	fio_shiftOut(_srDataRegister, _srDataMask, _srClockRegister, _srClockMask);
   
   
	// clock out SR data byte
	fio_shiftOut(_srDataRegister, _srDataMask, _srClockRegister, _srClockMask, val, MSBFIRST);
   
 	
	// strobe LCD enable which can now be toggled by the data line
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		fio_digitalWrite_HIGH(_srDataRegister, _srDataMask);
		waitUsec (1);         // enable pulse must be >450ns               
		fio_digitalWrite_SWITCHTO(_srDataRegister, _srDataMask, LOW);
	} // end critical section
}

// PUBLIC METHODS
// ---------------------------------------------------------------------------


/************ low level data pushing commands **********/
//
// send
void LiquidCrystal_SR2W::send(uint8_t value, uint8_t mode)
{
	uint8_t myMode = ( mode == DATA ) ? SR2W_RS_MASK : 0; 
   
	myMode = myMode | SR2W_EN_MASK | _blMask;

	if ( mode != FOUR_BITS )
	{
		loadSR(myMode | ((value >> 1) & SR2W_DATA_MASK)); // upper nibble
	}

	loadSR(myMode | ((value << 3) & SR2W_DATA_MASK)); // lower nibble
   
	/*
	 * Don't call waitUsec()
	 * do our own delay optmization since this code is so fast it needs some added delay
	 * even on slower AVRs.
	 */
#if (F_CPU <= 16000000)
	delayMicroseconds ( 10 );      // commands & data writes need > 37us to complete
#else
	delayMicroseconds ( 37 );      // commands & data writes need > 37us to complete
#endif
}

//
// setBacklight
void LiquidCrystal_SR2W::setBacklight ( uint8_t value ) 
{ 
	// Check for polarity to configure mask accordingly
	// ----------------------------------------------------------
	if  ( ((_blPolarity == POSITIVE) && (value > 0)) || 
        ((_blPolarity == NEGATIVE ) && ( value == 0 )) )
	{
		_blMask = SR2W_BL_MASK;
	}
	else 
	{
		_blMask = 0;
	}
   
	// send dummy data of blMask to set BL pin
	// Note: loadSR() will strobe the data line trying to pulse EN
	// but E will not strobe because the EN output bit is not set.
	loadSR(_blMask); 
}
