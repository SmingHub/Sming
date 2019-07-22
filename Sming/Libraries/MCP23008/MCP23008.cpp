// MCP23008.cpp
//------------------------------------------------------------------------------
// Library for communicating with MCP23008  8-bit port expander.
// Created by Garrett Blanton January, 24, 2013.
// Released into the public domain.
//
// Functions
//------------------------------------------------------------------------------
//	MCP23008(uint8_t addr);					// Class constructor
//	void writeIODIR(uint8_t cIODIR);		// Write IODIR register
//	void writeIPOL(uint8_t cIPOL);			// Write IPOL register
//	void writeGPINTEN(uint8_t cGPINTEN);	// Write GPINTEN register
//	void writeDEFVAL(uint8_t cDEFVAL);		// Write DEFVAL register
//	void writeGPPU(uint8_t cGPPU);			// Write GPPU register
//	void writeGPIO(uint8_t cGPIO);			// Write GPIO register
//	void writeOLAT(uint8_t cOLAT);			// Write OLAT register
//	uint8_t readIODIR();					// Read IODIR register
//	uint8_t readIPOL();						// Read IPOL register
//	uint8_t readGPINTEN();					// Read GPINTEN register
//	uint8_t readDEFVAL();					// Read DEFVAL register
//	uint8_t readGPPU();						// Read GPPU register
//	uint8_t readINTF();						// Read INTF register
//	uint8_t readINTCAP();					// Read INTCAP register
//	uint8_t readGPIO();						// Read GPIO register
//	uint8_t readOLAT();						// Read OLAT register
//
//	void writebyte(uint8_t Address, uint8_t Register, uint8_t Value);
//	uint8_t readbyte(uint8_t Address, uint8_t Register);


#include "Arduino.h"
#include "Wire.h"
#include "MCP23008.h"

MCP23008::MCP23008(byte addr)
{
  _ADDR = addr >> 1;
}

// writebyte
//------------------------------------------------------------------------------
// Writes a single byte of data to the specified MCP23008 register.
//
// Parameter	Description
//------------------------------------------------------------------------------
// Address		MCP23008 Address
// Register		MCP23008 Register to write
// Value		Value to write to register
//
// Registers
//------------------------------------------------------------------------------
// regIODIR      0x00	IO Direction Register
// regIPOL       0x01   Input Polarity Register
// regGPINTEN    0x02   Interrupt-On-Change Register
// regDEFVAL     0x03   Default Value Register
// regINTCON     0x04   Interrupt Control Register
// regIOCON      0x05   Configuration Register
// regGPPU       0x06   Pull-up Register
// regINTF       0x07   Interrupt Flag Register
// regINTCAP     0x08   Interrupt Captured Value Register
// regGPIO       0x09   General Purpose IO Register
// regOLAT       0x0A   Output Latch Register

void MCP23008::writebyte(uint8_t Address, uint8_t Register, uint8_t Value)
{
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Value);
  Wire.endTransmission();
}

// readbyte
//------------------------------------------------------------------------------
// Reads a single byte of data from the specified MCP23008 register.
//
// Parameter	Description
//------------------------------------------------------------------------------
// Address		MCP23008 Address
// Register		MCP23008 Register to write
// return		Value read from register
//
// Registers
//------------------------------------------------------------------------------
// regIODIR      0x00	IO Direction Register
// regIPOL       0x01   Input Polarity Register
// regGPINTEN    0x02   Interrupt-On-Change Register
// regDEFVAL     0x03   Default Value Register
// regINTCON     0x04   Interrupt Control Register
// regIOCON      0x05   Configuration Register
// regGPPU       0x06   Pull-up Register
// regINTF       0x07   Interrupt Flag Register
// regINTCAP     0x08   Interrupt Captured Value Register
// regGPIO       0x09   General Purpose IO Register
// regOLAT       0x0A   Output Latch Register

uint8_t MCP23008::readbyte(uint8_t Address, uint8_t Register)
{
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();
  Wire.requestFrom((char)Address,1);
  Wire.available();
  return Wire.read();
}

// writeIODIR
//------------------------------------------------------------------------------
// Writes to the MCP23008 IODIR register.
//
// Parameter	Description
//------------------------------------------------------------------------------
// cIODIR		Data direction value (0=output; 1=input)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				IO7 |IO6 |IO5 |IO4 |IO3 |IO2 |IO1 |IO0
  

void MCP23008::writeIODIR(uint8_t cIODIR)
{
  writebyte(_ADDR, regIODIR, cIODIR);
}

// writeIPOL
//------------------------------------------------------------------------------
// Writes to the MCP23008 IPOL register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// cIPOL		Input Polarity Register (0=Direct; 1=Opposite)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				IP7 |IP6 |IP5 |IP4 |IP3 |IP2 |IP1 |IP0

void MCP23008::writeIPOL(uint8_t cIPOL)
{
  writebyte(_ADDR, regIPOL, cIPOL);
}

// writeGPINTEN
//------------------------------------------------------------------------------
// Writes to the MCP23008 GPINTEN register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// cGPINTEN		Interrupt-On-Change Control Register (0=Disable; 1=Enable)
//
//				BIT7  |BIT6  |BIT5  |BIT4  |BIT3  |BIT2  |BIT1  |BIT0
//				GPINT7|GPINT6|GPINT5|GPINT4|GPINT3|GPINT2|GPINT1|GPINT0

void MCP23008::writeGPINTEN(uint8_t cGPINTEN)
{
  writebyte(_ADDR, regGPINTEN, cGPINTEN);
}

// writeDEFVAL
//------------------------------------------------------------------------------
// Writes to the MCP23008 DEFVAL register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// cDEFVAL		Default Compare Register For Interrupt-On-Change (GPINT!=DEF)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				DEF7|DEF6|DEF5|DEF4|DEF3|DEF2|DEF1|DEF0

void MCP23008::writeDEFVAL(uint8_t cDEFVAL)
{
  writebyte(_ADDR, regDEFVAL, cDEFVAL);
}

// writeINTCON
//------------------------------------------------------------------------------
// Writes to the MCP23008 INTCON register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// cINTCON		Interrupt Control Register (0=Compare Previous Pin; 1=DEFVAL)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				IOC7|IOC6|IOC5|IOC4|IOC3|IOC2|IOC1|IOC0

void MCP23008::writeINTCON(uint8_t cINTCON)
{
  writebyte(_ADDR, regINTCON, cINTCON);
}

// writeIOCON
//------------------------------------------------------------------------------
// Writes to the MCP23008 IOCON register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// cIOCON		Configuration Register
//
//				Bit5	0=Sequential Operation Enabled;
//						1=Sequential Operation Disabled
//				Bit4	0=Slew rate enabled
//						1=Slew rate disabled
//				Bit2	0=Open-drain output
//						1=Active driver output
//				Bit1	0=INT output active low
//						1=INT output active high

void MCP23008::writeIOCON(uint8_t cIOCON)
{
  writebyte(_ADDR, regIOCON, cIOCON);
}

// writeGPPU
//------------------------------------------------------------------------------
// Writes to the MCP23008 GPPU register.
//
// Parameter	Description
//------------------------------------------------------------------------------
// cGPPU		Pull-up resister configuration (0=Disabled; 1=Enabled)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				PU7 |PU6 |PU5 |PU4 |PU3 |PU2 |PU1 |PU0  

void MCP23008::writeGPPU(uint8_t cGPPU)
{
  writebyte(_ADDR, regGPPU, cGPPU);
}

// writeGPIO
//------------------------------------------------------------------------------
// Writes to the MCP23008 GPIO register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// cGPIO		Port Register (0=Logic Low; 1=Logic High)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				GP7 |GP6 |GP5 |GP4 |GP3 |GP2 |GP1 |GP0  

void MCP23008::writeGPIO(uint8_t cGPIO)
{
  writebyte(_ADDR, regGPIO, cGPIO);
}

// writeOLAT
//------------------------------------------------------------------------------
// Writes to the MCP23008 OLAT register.
//
// Parameter	Description
//------------------------------------------------------------------------------
// cGPIO		Output Latch Register (0=Logic Low; 1=Logic High)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				OL7 |OL6 |OL5 |OL4 |OL3 |OL2 |OL1 |OL0  

void MCP23008::writeOLAT(uint8_t cOLAT)
{
  writebyte(_ADDR, regOLAT, cOLAT);
}

// readIODIR
//------------------------------------------------------------------------------
// Reads from the MCP23008 IODIR register.
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Data direction value (0=output; 1=input)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				GP7 |GP6 |GP5 |GP4 |GP3 |GP2 |GP1 |GP0  

uint8_t MCP23008::readIODIR()
{
  return readbyte(_ADDR, regIODIR);
}

// readIPOL
//------------------------------------------------------------------------------
// Reads from the MCP23008 IPOL register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Input Polarity Register (0=Direct; 1=Opposite)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				IP7 |IP6 |IP5 |IP4 |IP3 |IP2 |IP1 |IP0

uint8_t MCP23008::readIPOL()
{
  return readbyte(_ADDR, regIPOL);
}

// readGPINTEN
//------------------------------------------------------------------------------
// Reads from the MCP23008 GPINTEN register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Interrupt-On-Change Control Register (0=Disable; 1=Enable)
//
//				BIT7  |BIT6  |BIT5  |BIT4  |BIT3  |BIT2  |BIT1  |BIT0
//				GPINT7|GPINT6|GPINT5|GPINT4|GPINT3|GPINT2|GPINT1|GPINT0

uint8_t MCP23008::readGPINTEN()
{
  return readbyte(_ADDR, regGPINTEN);
}

// readDEFVAL
//------------------------------------------------------------------------------
// Reads from the MCP23008 DEFVAL register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Default Compare Register For Interrupt-On-Change (GPINT!=DEF)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				DEF7|DEF6|DEF5|DEF4|DEF3|DEF2|DEF1|DEF0

uint8_t MCP23008::readDEFVAL()
{
  return readbyte(_ADDR, regDEFVAL);
}

// readINTCON
//------------------------------------------------------------------------------
// Reads from the MCP23008 INTCON register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Interrupt Control Register (0=Compare Previous Pin; 1=DEFVAL)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				IOC7|IOC6|IOC5|IOC4|IOC3|IOC2|IOC1|IOC0

uint8_t MCP23008::readINTCON()
{
  return readbyte(_ADDR, regINTCON);
}

// readIOCON
//------------------------------------------------------------------------------
// Reads from the MCP23008 IOCON register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Configuration Register
//
//				Bit5	0=Sequential Operation Enabled;
//						1=Sequential Operation Disabled
//				Bit4	0=Slew rate enabled
//						1=Slew rate disabled
//				Bit2	0=Open-drain output
//						1=Active driver output
//				Bit1	0=INT output active low
//						1=INT output active high

uint8_t MCP23008::readIOCON()
{
  return readbyte(_ADDR, regIOCON);
}

// readGPPU
//------------------------------------------------------------------------------
// Reads from the MCP23008 GPPU register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Pull-up resister configuration (0=Disabled; 1=Enabled)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				PU7 |PU6 |PU5 |PU4 |PU3 |PU2 |PU1 |PU0  

uint8_t MCP23008::readGPPU()
{
  return readbyte(_ADDR, regGPPU);
}

// readINTF
//------------------------------------------------------------------------------
// Reads from the MCP23008 INTF register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Interrupt Flag Register (0=No Interrupt; 1=Caused Interrupt)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				INT7|INT6|INT5|INT4|INT3|INT2|INT1|INT0  

uint8_t MCP23008::readINTF()
{
  return readbyte(_ADDR, regINTF);
}

// readINTCAP
//------------------------------------------------------------------------------
// Reads from the MCP23008 INTCAP register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// return		Interrupt Capture Register (0=Logic Low; 1=Logic High)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				INT7|INT6|INT5|INT4|INT3|INT2|INT1|INT0  

uint8_t MCP23008::readINTCAP()
{
  return readbyte(_ADDR, regINTCAP);
}

// readGPIO
//------------------------------------------------------------------------------
// Reads from the MCP23008 GPIO register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// cGPIO		Port Register (0=Logic Low; 1=Logic High)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				GP7 |GP6 |GP5 |GP4 |GP3 |GP2 |GP1 |GP0

uint8_t MCP23008::readGPIO()
{
  return readbyte(_ADDR, regGPIO);
}

// readOLAT
//------------------------------------------------------------------------------
// Reads from the MCP23008 OLAT register.  
//
// Parameter	Description
//------------------------------------------------------------------------------
// cGPIO		Output Latch Register (0=Logic Low; 1=Logic High)
//
//				BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
//				OL7 |OL6 |OL5 |OL4 |OL3 |OL2 |OL1 |OL0  

uint8_t MCP23008::readOLAT()
{
  return readbyte(_ADDR, regOLAT);
}
