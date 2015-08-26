// --------------------------------------
// i2c_scanner (Sming version)
//
// Version 1
//    This program (or code that looks like it)
//    can be found in many places.
//    For example on the Arduino.cc forum.
//    The original author is not know.
// Version 2, Juni 2012, Using Arduino 1.0.1
//     Adapted to be as simple as possible by Arduino.cc user Krodal
// Version 3, Feb 26  2013
//    V3 by louarnold
// Version 4, March 3, 2013, Using Arduino 1.0.3
//    by Arduino.cc user Krodal.
//    Changes by louarnold removed.
//    Scanning addresses changed from 0...127 to 1...119,
//    according to the i2c scanner by Nick Gammon
//    http://www.gammon.com.au/forum/?id=10896
// Version 5, March 28, 2013
//    As version 4, but address scans now to 127.
//    A sensor seems to use address 120.
// Version 6, March 15  2015
//    Sming version by Skurydin Alexey
//
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//

#include "../../Sming/SmingCore/HardwareSerial.h"
#include "../../Sming/SmingCore/Platform/WDT.h"
#include "../../Sming/SmingCore/Timer.h"
#include "../../Sming/SmingCore/Wire.h"
#include "../../Sming/Wiring/WConstants.h"
#include "../include/user_config.h"

Timer procTimer;

void scanBus()
{
	byte error, address;
	int nDevices;

	Serial.println("Scanning...");

	nDevices = 0;
	for (address = 1; address < 127; address++)
	{
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.
		Wire.beginTransmission(address);
		error = Wire.endTransmission();

		WDT.alive(); // Second option: notify Watch Dog what you are alive (feed it)

		if (error == 0)
		{
			Serial.print("I2C device found at address 0x");
			if (address < 16)
				Serial.print("0");
			Serial.print(address, HEX);
			Serial.println("  !");

			nDevices++;
		}
		else if (error == 4)
		{
			Serial.print("Unknow error at address 0x");
			if (address < 16)
				Serial.print("0");
			Serial.println(address, HEX);
		}
	}
	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
	else
	{
		Serial.println("done scanning\n");

		unsigned char Setup[4];
		unsigned char Data[3];
		/********************************************************************
		 DATA AQUISITION LOOP
		 ********************************************************************/
		// The loop performes a single convertion and reads out the convertion results
		while (1)
		{            // Endless loop

			// START CONVERSION ON AD774x
			// ==============================
			Setup[0] = 0xA0;  // (Cap Setup Reg) Enable Cap Channel , CAPDIFF on
			Setup[1] = 0x81; // (VT Setup Reg) Enable Voltage/Temperature, Chop on
			Setup[2] = 0x0E;  // (EXC Setup Reg) Enable Exc, Level = Vdd*3/8
			Setup[3] = 0xA2; // (Configuration Reg) Start single conversion, Default rate
			I2C_Snd(1, 0x48, 0x07, 4, Setup); // Send setup to AD7747
			// Send to I2C:
			// 1    .. Transaction with 1-byte address pointer
			// 0x90 .. AD774x device address
			// 0x07 .. starting adress pointer - AD774x Cap Setup register
			// 4    .. 4 bytes to be sent
			// Setup .. array of bytes to be sent

			// WAIT FOR AD774x RDY
			do
			{
				I2C_Rcv(1, 0x48, 0x00, 1, Data);  // Read AD774x Status
			} while (Data[0] & 0x04);           // Wait for RDY bit = 0

			// READ AD774x DATA CAP CHANNEL
			I2C_Rcv(1, 0x90, 0x01, 3, Data);    // Read AD774x Data

			// SEND DATA TO UART
			// ==============================
			Serial.printf("%02bX%02bX%02bX\t", Data[0], Data[1], Data[2]);

			// READ AD774x DATA VT CHANNEL
			I2C_Rcv(1, 0x48, 0x04, 3, Data);    // Read AD774x Data

			// SEND DATA TO UART
			// ==============================
			Serial.printf("%02bX%02bX%02bX\t", Data[0], Data[1], Data[2]);

			// to convert the code to degC you have to calculate the following formula
			// degC = decimal(Data) / 2048 - 4096

		}  // while

	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output

	WDT.enable(false); // First (but not the best) option: fully disable watch dog timer

	// You can change pins:
	//Wire.pins(12, 14); // SCL, SDA
	Wire.pins(4, 5);
	Wire.begin();
	procTimer.initializeMs(3000, scanBus).start();
}

unsigned char I2C_Rcv_Byte(boolean noack)
// Receives one byte from I2C
//
// noack should be 0 if other bytes will be received
// i.e., slave will be sent ACK
//
// noack should be 1 if this is the last byte
// i.e., slave will be sent NACK
{
	unsigned char i;
	unsigned char d;

	MDE = 0;        // SDA input
	for (i = 8; i > 0; i--)
	{
		MCO = 1;      // SCL hi
		// consider using _crol_(d,1) ?
		d <<= 1;      // rotate data
		d |= MDI;     // read SDA -> data
		MCO = 0;      // SCL lo
	}
	MDE = 1;        // SDA output
	MDO = noack;    // SDA = ACK .. NACK for the last byte
	MCO = 1;        // SCL hi
	MCO = 0;        // SCL lo
	MDE = 0;        // SDA input - release
	return (d);

} // end of I2C_Rcv_Byte


bool I2C_Snd_Byte(unsigned char d)
// Sends one byte to I2C
//
// Returns 0 if byte is acknowledged by slave
// Returns 1 if transaction fails
{
  unsigned char i;
  unsigned char noack;

  MDE = 1;        // SDA output
  for (i=8; i>0; i--)
  {
    // consider _crol_(d,1) ?
    MDO = d >> 7; // send data -> SDA
    MCO = 1;      // SCL hi
    MCO = 0;      // SCL lo
    d <<= 1;      // rotate data
  }
  MDE = 0;        // SDA input
  MCO = 1;        // SCL hi
  noack = MDI;      // check SDA for ACK
  MCO = 0;        // SCL lo
//  return(noack);

  return(0); // *************************** FOR DEBUG - NO ACK CHECK !

} // end of I2C_Snd_Byte


unsigned char I2C_Snd(unsigned char Type, unsigned char Adr, unsigned int Ptr,
                      unsigned char Length, unsigned char* Buffer)
{
  unsigned char i;

  //I2C_Start();                  // Start I2C transaction
  Wire.beginTransmission(0x48);

  if (I2C_Snd_Byte(Adr & 0xFE)) // Send slave address, write
  {
    // if NACK, terminate transaction
    I2C_Stop();
    return(1);
  }

  if (Type > 1)                 // Check if 16-bit pointer is used
  {
    if (I2C_Snd_Byte(Ptr >> 8)) // Send high byte of pointer
    {
      // if NACK, terminate transaction
      I2C_Stop();
      return(1);
    }
  }

  if (Type > 0)                 // Check if pointer is used
  {
    if (I2C_Snd_Byte(Ptr & 0x00FF)) // Send pointer (low byte)
    {
      // if NACK, terminate transaction
      I2C_Stop();
      return(1);
    }
  }

  for (i=0; i<Length; i++)
  {
    if (I2C_Snd_Byte(*(Buffer+i)))    // Send data
    {
      // if NACK, terminate transaction
      I2C_Stop();
      return(1);
    }
  }

  I2C_Stop();                   // Stop I2C transaction
  return(0);                    // Confirm no error

} // end of I2C_Snd

unsigned char I2C_Rcv(unsigned char Type, unsigned char Adr, unsigned int Ptr,
                      unsigned char Length, unsigned char* Buffer)
{
  unsigned char i;

  if (!Length) return(1);       // Length MUST be > 0 !!!

  // ----------------------------------------------------------------
  // This code is executed only if the I2C pointer is used
  // ----------------------------------------------------------------

  if (Type > 0)                 // Check if pointer is used
  {
    I2C_Start();                  // Start I2C transaction

    if (I2C_Snd_Byte(Adr & 0xFE)) // Send slave address, write
    {
      // if NACK, terminate transaction
      I2C_Stop();
      return(1);
    }

    if (Type > 1)                 // Check if 16-bit pointer is used
    {
      if (I2C_Snd_Byte(Ptr >> 8)) // Send high byte of pointer
      {
        // if NACK, terminate transaction
        I2C_Stop();
        return(1);
      }
    }

    if (I2C_Snd_Byte(Ptr & 0xFF)) // Send pointer (low byte)
    {
      // if NACK, terminate transaction
      I2C_Stop();
      return(1);
    }
  }

  // ----------------------------------------------------------------
  // From here the code is executed regardless on the I2C pointer
  // ----------------------------------------------------------------

  I2C_Start();                  // Start I2C transaction

  if (I2C_Snd_Byte(Adr | 0x01)) // Send slave address, read
  {
    // if NACK, terminate transaction
    I2C_Stop();
    return(1);
  }

  // Cycle through data except the last byte
  Length--;
  for (i=0; i<Length; i++)
  {
    *(Buffer+i) = I2C_Rcv_Byte(0);  // Receive data and do ACK
  }

  // The last byte
  *(Buffer+i) = I2C_Rcv_Byte(1);  // Receive data and do NOT ACK

  I2C_Stop();                   // Stop I2C transaction
  return(0);                    // Confirm no error

} // end of I2C_Rcv

//*******************************************************************

