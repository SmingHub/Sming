/*
 MCP23S17.h  Version 0.1
 Microchip MCP23S17 SPI I/O Expander Class for Arduino
 Created by Cort Buffington & Keith Neufeld
 March, 2011

 Features Implemented (by word and bit):
 I/O Direction
 Pull-up on/off
 Input inversion
 Output write
 Input read

 Interrupt features are not implemented in this version
 byte based (portA, portB) functions are not implemented in this version

 NOTE:  Addresses below are only valid when IOCON.BANK=0 (register addressing mode)
 This means one of the control register values can change register addresses!
 The default values is 0, so that's how we're using it.

 All registers except ICON (0xA and 0xB) are paired as A/B for each 8-bit GPIO port.
 Comments identify the port's name, and notes on how it is used.

 *THIS CLASS ENABLES THE ADDRESS PINS ON ALL CHIPS ON THE BUS WHEN THE FIRST CHIP OBJECT IS INSTANTIATED!

 USAGE: All Read/Write functions except wordWrite are implemented in two different ways.
 Individual pin values are set by referencing "pin #" and On/Off, Input/Output or High/Low where
 portA represents pins 0-7 and portB 8-15. So to set the most significant bit of portB, set pin # 15.
 To Read/Write the values for the entire chip at once, a word mode is supported buy passing a
 single argument to the function as 0x(portB)(portA). I/O mode Output is represented by 0.
 The wordWrite function was to be used internally, but was made public for advanced users to have
 direct and more efficient control by writing a value to a specific register pair.
 */

#ifndef MCP23S17_h
#define MCP23S17_h

// REGISTERS ARE DEFINED HERE SO THAT THEY MAY BE USED IN THE MAIN PROGRAM

#define    IODIRA    (0x00)      // MCP23x17 I/O Direction Register
#define    IODIRB    (0x01)      // 1 = Input (default), 0 = Output

#define    IPOLA     (0x02)      // MCP23x17 Input Polarity Register
#define    IPOLB     (0x03)      // 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)

#define    GPINTENA  (0x04)      // MCP23x17 Interrupt on Change Pin Assignements
#define    GPINTENB  (0x05)      // 0 = No Interrupt on Change (default), 1 = Interrupt on Change

#define    DEFVALA   (0x06)      // MCP23x17 Default Compare Register for Interrupt on Change
#define    DEFVALB   (0x07)      // Opposite of what is here will trigger an interrupt (default = 0)

#define    INTCONA   (0x08)      // MCP23x17 Interrupt on Change Control Register
#define    INTCONB   (0x09)      // 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)

#define    IOCON     (0x0A)      // MCP23x17 Configuration Register
//                   (0x0B)      //     Also Configuration Register

#define    GPPUA     (0x0C)      // MCP23x17 Weak Pull-Up Resistor Register
#define    GPPUB     (0x0D)      // INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up 

#define    INTFA     (0x0E)      // MCP23x17 Interrupt Flag Register
#define    INTFB     (0x0F)      // READ ONLY: 1 = This Pin Triggered the Interrupt

#define    INTCAPA   (0x10)      // MCP23x17 Interrupt Captured Value for Port Register
#define    INTCAPB   (0x11)      // READ ONLY: State of the Pin at the Time the Interrupt Occurred

#define    GPIOA     (0x12)      // MCP23x17 GPIO Port Register
#define    GPIOB     (0x13)      // Value on the Port - Writing Sets Bits in the Output Latch

#define    OLATA     (0x14)      // MCP23x17 Output Latch Register
#define    OLATB     (0x15)      // 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value!

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class MCP
{
public:
	MCP(uint8_t, uint8_t);// Constructor to instantiate a discrete IC as an object, arguments: the address (0-7) of the MCP23S17, CS ChipSelect pin
	void begin();		// Configure CS pin and enable address support on chip
	void wordWrite(uint8_t, unsigned int); // Typically only used internally, but allows the user to write any register pair if needed, so it's public
	void byteWrite(uint8_t, uint8_t); // Typically only used internally, but allows the user to write any register if needed, so it's public
	void pinMode(uint8_t, uint8_t); // Sets the mode (input or output) of a single I/O pin
	void pinMode(unsigned int); // Sets the mode (input or output) of all I/O pins at once
	void pullupMode(uint8_t, uint8_t); // Selects internal 100k input pull-up of a single I/O pin
	void pullupMode(unsigned int); // Selects internal 100k input pull-up of all I/O pins at once
	void inputInvert(uint8_t, uint8_t); // Selects input state inversion of a single I/O pin (writing 1 turns on inversion)
	void inputInvert(unsigned int); // Selects input state inversion of all I/O pins at once (writing a 1 turns on inversion)
	void digitalWrite(uint8_t, uint8_t); // Sets an individual output pin HIGH or LOW
	void digitalWrite(unsigned int); // Sets all output pins at once. If some pins are configured as input, those bits will be ignored on write
	uint8_t digitalRead(uint8_t);            // Reads an individual input pin
	uint8_t byteRead(uint8_t); // Reads an individual register and returns the byte. Argument is the register address
	unsigned int digitalRead(void); // Reads all input  pins at once. Be sure it ignore the value of pins configured as output!
private:
	uint8_t _address;                        // Address of the MCP23S17 in use
	uint8_T _cs;							// CS pin number for MCP23S17
	unsigned int _modeCache; // Caches the mode (input/output) configuration of I/O pins
	unsigned int _pullupCache; // Caches the internal pull-up configuration of input pins (values persist across mode changes)
	unsigned int _invertCache; // Caches the input pin inversion selection (values persist across mode changes)
	unsigned int _outputCache;            // Caches the output pin state of pins
};

#endif //MCP23S17
