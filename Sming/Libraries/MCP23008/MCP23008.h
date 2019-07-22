// MCP23008.h
//------------------------------------------------------------------------------
// Library for communicating with MCP23008 8-bit port expander.
// Created by Garrett Blanton January, 24, 2014.
// Released into the public domain.

#ifndef MCP23008_h
#define MCP23008_h

#include "Arduino.h"

#define regIODIR      0x00         // IO Direction Register
#define regIPOL       0x01         // Input Polarity Register
#define regGPINTEN    0x02         // Interrupt-On-Change Register
#define regDEFVAL     0x03         // Default Value Register
#define regINTCON     0x04         // Interrupt Control Register
#define regIOCON      0x05         // Configuration Register
#define regGPPU       0x06         // Pull-up Register
#define regINTF       0x07         // Interrupt Flag Register
#define regINTCAP     0x08         // Interrupt Captured Value Register
#define regGPIO       0x09         // General Purpose IO Register
#define regOLAT       0x0A         // Output Latch Register

class MCP23008
{
  public:
  
	MCP23008(uint8_t addr);					// Class constructor
	
	void writeIODIR(uint8_t cIODIR);		// Write IODIR register
	void writeIPOL(uint8_t cIPOL);			// Write IPOL register
	void writeGPINTEN(uint8_t cOLINTEN);	// Write GPINTEN register
	void writeDEFVAL(uint8_t cDEFVAL);		// Write DEFVAL register
	void writeINTCON(uint8_t cINTCON);		// Write INTCON register
	void writeIOCON(uint8_t cIOCON);		// Write IOCON register
	void writeGPPU(uint8_t cOLPU);			// Write GPPU register
	void writeGPIO(uint8_t cOLIO);			// Write GPIO register
	void writeOLAT(uint8_t cOLAT);			// Write OLAT register

	uint8_t readIODIR();					// Read IODIR register
	uint8_t readIPOL();						// Read IPOL register
	uint8_t readGPINTEN();					// Read GPINTEN register
	uint8_t readDEFVAL();					// Read DEFVAL register
	uint8_t readGPPU();						// Read GPPU register
	uint8_t readINTCON();					// Read INTCON register
	uint8_t readIOCON();					// Read IOCON register
	uint8_t readINTF();						// Read INTF register
	uint8_t readINTCAP();					// Read INTCAP register
	uint8_t readGPIO();						// Read GPIO register
	uint8_t readOLAT();						// Read OLAT register
	
  private:

	void writebyte(uint8_t Address, uint8_t Register, uint8_t Value);	// Writes byte to MCP23008
	uint8_t readbyte(uint8_t Address, uint8_t Register);				// Reads byte from MCP23008
  
	uint8_t _ADDR;							// MCP23008 Address : 0b0100xxx0
};

#endif