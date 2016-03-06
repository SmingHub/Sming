/*
 MCP23S17.cpp  Version 0.1
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

#include <SPI.h>                 // Arduino IDE SPI library - uses AVR hardware SPI features
#include "MCP23S17.h"            // Header files for this class
using namespace MCP23S17Registers;
using namespace MCP23S17Constants;

// Constructor to instantiate an instance of MCP to a specific chip (address)

MCP::MCP(uint8_t address, uint8_t cs)
{
	_cs = cs;
	_address = address;
	_rcmd = (OPCODER | (_address << 1)); //Read command for chip address
	_wcmd = (OPCODEW | (_address << 1)); //Write command for chip addres
	_modeCache = 0xFFFF;                // Default I/O mode is all input, 0xFFFF
	_outputCache = 0x0000;            // Default output state is all off, 0x0000
	_pullupCache = 0x0000;           // Default pull-up state is all off, 0x0000
	_invertCache = 0x0000; // Default input inversion state is not inverted, 0x0000
}

void MCP::begin()
{
	::pinMode(_cs, OUTPUT);
	::digitalWrite(_cs, HIGH);

	SPI.begin();                   // Start up the SPI bus� crank'er up Charlie!
	byteWrite(IOCON, ADDR_ENABLE);
}

// GENERIC BYTE WRITE - will write a byte to a register, arguments are register address and the value to write

void MCP::byteWrite(uint8_t reg, uint8_t value)
{      // Accept the register and byte
	::digitalWrite(_cs, LOW);
	SPI.transfer(_wcmd); // Send the MCP23S17 opcode, chip address, and write bit
	SPI.transfer(reg);                     // Send the register we want to write
	SPI.transfer(value);                                 // Send the byte
	::digitalWrite(_cs, HIGH);
}

// GENERIC WORD WRITE - will write a word to a register pair, LSB to first register, MSB to next higher value register

void MCP::wordWrite(uint8_t reg, unsigned int word)
{  // Accept the start register and word
	::digitalWrite(_cs, LOW);
	SPI.transfer(_wcmd); // Send the MCP23S17 opcode, chip address, and write bit
	SPI.transfer(reg);                    // Send the register we want to write
	SPI.transfer((uint8_t) (word)); // Send the low byte (register address pointer will auto-increment after write)
	SPI.transfer((uint8_t) (word >> 8)); // Shift the high byte down to the low byte location and send
	::digitalWrite(_cs, HIGH);
}

// MODE SETTING FUNCTIONS - BY PIN AND BY WORD

void MCP::pinMode(uint8_t pin, uint8_t mode)
{  // Accept the pin # and I/O mode
	if (pin < 1 | pin > 16)
		return; // If the pin value is not valid (1-16) return, do nothing and return
	if (mode == INPUT)
	{      // Determine the mode before changing the bit state in the mode cache
		_modeCache |= 1 << (pin - 1); // Since input = "HIGH", OR in a 1 in the appropriate place
	}
	else
	{
		_modeCache &= ~(1 << (pin - 1)); // If not, the mode must be output, so and in a 0 in the appropriate place
	}
	wordWrite(IODIRA, _modeCache); // Call the generic word writer with start register and the mode cache
}

void MCP::pinMode(unsigned int mode)
{    // Accept the word�
	wordWrite(IODIRA, mode); // Call the the generic word writer with start register and the mode cache
	_modeCache = mode;
}

// THE FOLLOWING WRITE FUNCTIONS ARE NEARLY IDENTICAL TO THE FIRST AND ARE NOT INDIVIDUALLY COMMENTED

// WEAK PULL-UP SETTING FUNCTIONS - BY WORD AND BY PIN

void MCP::pullupMode(uint8_t pin, uint8_t mode)
{
	if (pin < 1 | pin > 16)
		return;
	if (mode == ON)
	{
		_pullupCache |= 1 << (pin - 1);
	}
	else
	{
		_pullupCache &= ~(1 << (pin - 1));
	}
	wordWrite(GPPUA, _pullupCache);
}

void MCP::pullupMode(unsigned int mode)
{
	wordWrite(GPPUA, mode);
	_pullupCache = mode;
}

// INPUT INVERSION SETTING FUNCTIONS - BY WORD AND BY PIN

void MCP::inputInvert(uint8_t pin, uint8_t mode)
{
	if (pin < 1 | pin > 16)
		return;
	if (mode == ON)
	{
		_invertCache |= 1 << (pin - 1);
	}
	else
	{
		_invertCache &= ~(1 << (pin - 1));
	}
	wordWrite(IPOLA, _invertCache);
}

void MCP::inputInvert(unsigned int mode)
{
	wordWrite(IPOLA, mode);
	_invertCache = mode;
}

// WRITE FUNCTIONS - BY WORD AND BY PIN

void MCP::digitalWrite(uint8_t pin, uint8_t value)
{
	if (pin < 1 | pin > 16)
		return;
	if (pin < 1 | pin > 16)
		return;
	if (value)
	{
		_outputCache |= 1 << (pin - 1);
	}
	else
	{
		_outputCache &= ~(1 << (pin - 1));
	}
	wordWrite(GPIOA, _outputCache);
}

void MCP::digitalWrite(unsigned int value)
{
	wordWrite(GPIOA, value);
	_outputCache = value;
}

// READ FUNCTIONS - BY WORD, BYTE AND BY PIN

unsigned int MCP::digitalRead(void)
{ // This function will read all 16 bits of I/O, and return them as a word in the format 0x(portB)(portA)
	unsigned int value = 0; // Initialize a variable to hold the read values to be returned
	::digitalWrite(_cs, LOW);
	SPI.transfer(_rcmd); // Send the MCP23S17 opcode, chip address, and read bit
	SPI.transfer(GPIOA);                    // Send the register we want to read
	value = SPI.transfer(0x00); // Send any byte, the function will return the read value (register address pointer will auto-increment after write)
	value |= (SPI.transfer(0x00) << 8); // Read in the "high byte" (portB) and shift it up to the high location and merge with the "low byte"
	::digitalWrite(_cs, HIGH);
	return value; // Return the constructed word, the format is 0x(portB)(portA)
}

uint8_t MCP::byteRead(uint8_t reg)
{        // This function will read a single register, and return it
	uint8_t value = 0; // Initialize a variable to hold the read values to be returned
	::digitalWrite(_cs, LOW);
	SPI.transfer(_rcmd); // Send the MCP23S17 opcode, chip address, and read bit
	SPI.transfer(reg);                      // Send the register we want to read
	value = SPI.transfer(0x00); // Send any byte, the function will return the read value
	::digitalWrite(_cs, HIGH);
	return value; // Return the constructed word, the format is 0x(register value)
}

uint8_t MCP::digitalRead(uint8_t pin)
{              // Return a single bit value, supply the necessary bit (1-16)
	if (pin < 1 | pin > 16)
		return 0x0; // If the pin value is not valid (1-16) return, do nothing and return
	return digitalRead() & (1 << (pin - 1)) ? HIGH : LOW; // Call the word reading function, extract HIGH/LOW information from the requested pin
}
