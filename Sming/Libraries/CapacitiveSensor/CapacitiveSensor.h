/*
  CapacitiveSense.h v.04 - Capacitive Sensing Library for 'duino / Wiring
  https://github.com/PaulStoffregen/CapacitiveSensor
  http://www.pjrc.com/teensy/td_libs_CapacitiveSensor.html
  http://playground.arduino.cc/Main/CapacitiveSensor
  Copyright (c) 2008 Paul Bagder  All rights reserved.
  Version 05 by Paul Stoffregen - Support non-AVR board: Teensy 3.x, Arduino Due
  Version 04 by Paul Stoffregen - Arduino 1.0 compatibility, issue 146 fix
  vim: set ts=4:
*/

// ensure this library description is only included once
#ifndef CapacitiveSensor_h
#define CapacitiveSensor_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Direct I/O through registers and bitmask (from OneWire library)


#define PIN_TO_BASEREG(pin)             (portOutputRegister(digitalPinToPort(pin)))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define IO_REG_TYPE uint8_t
#define IO_REG_ASM
#define DIRECT_READ(base, mask)         (((*((base)+GPIO_IN_ADDRESS)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+GPIO_ENABLE_ADDRESS)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+GPIO_ENABLE_ADDRESS)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*(base)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*(base)) |= (mask))




// library interface description
class CapacitiveSensor
{
  // user-accessible "public" interface
  public:
  // methods
	CapacitiveSensor(uint8_t sendPin, uint8_t receivePin);
	long capacitiveSensorRaw(uint8_t samples);
	long capacitiveSensor(uint8_t samples);
	void set_CS_Timeout_Millis(unsigned long timeout_millis);
	void reset_CS_AutoCal();
	void set_CS_AutocaL_Millis(unsigned long autoCal_millis);
  // library-accessible "private" interface
  private:
  // variables
	int error;
	unsigned long  leastTotal;
	unsigned int   loopTimingFactor;
	unsigned long  CS_Timeout_Millis;
	unsigned long  CS_AutocaL_Millis;
	unsigned long  lastCal;
	unsigned long  total;
	IO_REG_TYPE sBit;   // send pin's ports and bitmask
	volatile IO_REG_TYPE *sReg;
	IO_REG_TYPE rBit;    // receive pin's ports and bitmask
	volatile IO_REG_TYPE *rReg;
  // methods
	int SenseOneCycle(void);
};

#endif
