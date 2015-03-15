// ---------------------------------------------------------------------------
// Created by Florian Fida on 20/01/12
// Copyright 2012 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//        http://creativecommons.org/licenses/by-sa/3.0/
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
// ---------------------------------------------------------------------------
// fio_shiftOut1 functions are based on Shif1 protocol developed by Roman Black 
// (http://www.romanblack.com/shift1.htm)
//
// Thread Safe: No
// Extendable: Yes
//
// @file FastIO.h
// This file implements basic fast IO routines.
// 
// @brief 
//
// @version API 1.0.0
//
// @author Florian Fida -
// 2012-03-16 bperrybap mods for chipkit32 (pic32) Arduino
//  support chipkit:
// (https://github.com/chipKIT32/chipKIT32-MAX/blob/master/hardware/pic32/
//   cores/pic32/wiring_digital.c)
// ---------------------------------------------------------------------------
#ifndef _FAST_IO_H_
#define _FAST_IO_H_

#if (ARDUINO <  100)
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include <pins_arduino.h> // pleasing sanguino core
#include <inttypes.h>


#define SKIP 0x23

#if defined (__AVR__)
#include <util/atomic.h> // for critical section management
typedef uint8_t fio_bit;
typedef volatile uint8_t *fio_register;


#elif defined(__PIC32MX__)
typedef uint32_t fio_bit;
typedef volatile uint32_t *fio_register;


#else
// fallback to Arduino standard digital i/o routines
#define FIO_FALLBACK
#define ATOMIC_BLOCK(dummy) if(true)
#define ATOMIC_RESTORESTATE
typedef uint8_t fio_bit;
typedef uint8_t fio_register;
#endif



#if !defined(FIO_FALLBACK) && !defined(ATOMIC_BLOCK)
/*
 * Define an ATOMIC_BLOCK that implements ATOMIC_FORCEON type
 * Using the portable Arduino interrupts() and noInterrupts()
 */
#define ATOMIC_RESTORESTATE ATOMIC_FORCEON // sorry, no support for save/restore yet.
#define ATOMIC_FORCEON uint8_t sreg_save \
              __attribute__((__cleanup__(__iSeiParam))) = 0

static __inline__ uint8_t __iCliRetVal(void)
{
	noInterrupts();
	return(1);
}
static __inline__ void __iSeiParam(const uint8_t *__s)
{
	interrupts();
}
#define ATOMIC_BLOCK(type) for(type,  __Todo = __iCliRetVal(); __Todo; __Todo = 0)

#endif // end of block to create compatible ATOMIC_BLOCK()



/*!
 @function
 @abstract  Get the output register for specified pin.
 @discussion if fast digital IO is disabled this function returns NULL
 @param  pin[in] Number of a digital pin
 @result  Register
 */
fio_register fio_pinToOutputRegister(uint8_t pin, uint8_t initial_state = LOW);

/*!
 @function
 @abstract  Get the input register for specified pin.
 @discussion if fast digital IO is disabled this function returns NULL
 @param  pin[in] Number of a digital pin
 @result  Register
 */
fio_register fio_pinToInputRegister(uint8_t pin);

/*!
 @function
 @abstract Find the bit which belongs to specified pin
 @discussion if fast digitalWrite is disabled this function returns the pin
 @param pin[in] Number of a digital pin
 @result Bit
 */
fio_bit fio_pinToBit(uint8_t pin);


/*!
 @method
 @abstract direct digital write
 @discussion without any checks
 @discussion falls back to normal digitalWrite if fast io is disabled
 @param pinRegister[in] Register - ignored if fast digital write is disabled
 @param pinBit[in] Bit - Pin if fast digital write is disabled
 @param value[in] desired output
 */
// __attribute__ ((always_inline)) /* let the optimizer decide that for now */
void fio_digitalWrite ( fio_register pinRegister, fio_bit pinBit, uint8_t value );

/**
 * This is where the magic happens that makes things fast.
 * Implemented as preprocessor directives to force inlining
 * SWITCH is fast for FIO but probably slow for FIO_FALLBACK so SWITCHTO is recommended if the value is known.
 */

#ifndef FIO_FALLBACK
#define fio_digitalWrite_LOW(reg,bit) *reg &= ~bit
#define fio_digitalWrite_HIGH(reg,bit) *reg |= bit
#define fio_digitalWrite_SWITCH(reg,bit) *reg ^= bit
#define fio_digitalWrite_SWITCHTO(reg,bit,val) fio_digitalWrite_SWITCH(reg,bit)
#else
// reg -> dummy NULL, bit -> pin
#define fio_digitalWrite_HIGH(reg,bit) digitalWrite(bit,HIGH)
#define fio_digitalWrite_LOW(reg,bit) digitalWrite(bit,LOW)
#define fio_digitalWrite_SWITCH(reg,bit) digitalWrite(bit, !digitalRead(bit))
#define fio_digitalWrite_SWITCHTO(reg,bit,val) digitalWrite(bit,val);
#endif

/*!
 @function
 @abstract direct digital read
 @discussion without any checks
 @discussion falls back to normal digitalRead if fast io is disabled
 @param pinRegister[in] Register - ignored if fast io is disabled
 @param pinBit[in] Bit - Pin if fast io is disabled
 @result Value read from pin
 */
int fio_digitalRead ( fio_register pinRegister, fio_bit pinBit );

/*!
 @method
 @abstract faster shift out
 @discussion using fast digital write
 @discussion falls back to normal digitalWrite if fastio is disabled
 @param dataRegister[in] Register of data pin - ignored if fast digital write is disabled
 @param dataBit[in] Bit of data pin - Pin if fast digital write is disabled
 @param clockRegister[in] Register of data pin - ignored if fast digital write is disabled
 @param clockBit[in] Bit of data pin - Pin if fast digital write is disabled
 @param bitOrder[in] bit order
 */
void fio_shiftOut( fio_register dataRegister, fio_bit dataBit, fio_register clockRegister, 
                  fio_bit clockBit, uint8_t value, uint8_t bitOrder );

/*!
 @method
 @abstract faster shift out clear
 @discussion using fast digital write
 @discussion falls back to normal digitalWrite if fastio is disabled
 @param dataRegister[in] Register of data pin - ignored if fast digital write is disabled
 @param dataBit[in] Bit of data pin - Pin if fast digital write is disabled
 @param clockRegister[in] Register of data pin - ignored if fast digital write is disabled
 @param clockBit[in] Bit of data pin - Pin if fast digital write is disabled
 */
void fio_shiftOut(fio_register dataRegister, fio_bit dataBit, fio_register clockRegister, fio_bit clockBit);

/*!
 * @method
 * @abstract one wire shift out
 * @discussion protocol needs initialisation (fio_shiftOut1_init)
 * @param shift1Register[in] pins register
 * @param shift1Bit[in] pins bit
 * @param value[in] value to shift out, last byte is ignored and always shifted out LOW
 */
void fio_shiftOut1(fio_register shift1Register, fio_bit shift1Bit, uint8_t value, boolean noLatch = false);
/*!
 * @method
 * @abstract one wire shift out
 * @discussion protocol needs initialisation (fio_shiftOut1_init)
 * @param pin[in] digital pin
 * @param value[in] value to shift out, last byte is ignored and always shifted out LOW
 */
void fio_shiftOut1(uint8_t pin, uint8_t value, boolean noLatch = false);
/*!
 * @method
 * @abstract initializes one wire shift out protocol
 * @discussion Puts pin to HIGH state and delays until Capacitors are charged.
 * @param shift1Register[in] pins register
 * @param shift1Bit[in] pins bit
 */
void fio_shiftOut1_init(fio_register shift1Register, fio_bit shift1Bit);
/*!
 * @method
 * @abstract initializes one wire shift out protocol
 * @discussion Puts pin to HIGH state and delays until Capacitors are charged.
 * @param pin[in] digital pin
 */
void fio_shiftOut1_init(uint8_t pin);

#endif // FAST_IO_H
