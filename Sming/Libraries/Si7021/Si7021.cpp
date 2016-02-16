#include "Si7021.h"
#include <Wire.h>

#define RH_CALC(n)		(((125 * (float) (n)) / 65536) - 6)
#define TEMP_CALC(n)	(((175.72f * (n)) / 65536) - 46.85f)

#define SI7021_ADDR		0x40

SI7021::SI7021(int pinSCL, int pinSDA)
{
	SCL = pinSCL;
	SDA = pinSDA;
}

void SI7021::begin()
{
	Wire.pins(SCL,SDA);
	Wire.begin();											// Begin I2C
    Wire.beginTransmission(SI7021_ADDR);
    present = (Wire.endTransmission() == 0);				// check if present
	reset();												// Reset sensor
}

bool SI7021::isPresent() {
    return present;
}

float SI7021::readHumidity()
{
    return RH_CALC(readSensor(0xE5)); 						// Return humidity
}

float SI7021::readTemp()
{
	return TEMP_CALC(readSensor(0xE3));						// Return temp
}

float SI7021::readTempPrev()
{
	return TEMP_CALC(readSensor(0xE0));						// Return previous temp
}

void SI7021::setHumidityRes(uint8_t res)
{
	uint8_t uReg = readRegister(0xE7); 						// Read user register
		if(res == 12)	uReg &= ~(0x81);					// Set 12-bit humidity
		if(res == 11)	uReg |= (0x81);						// Set 11-bit humidity
		if(res == 10)	uReg = (uReg & ~(0x81)) | (0x80);	// Set 10-bit humidity
		if(res ==  8)	uReg = (uReg & ~(0x81)) | (0x01);	// Set  8-bit humidity
	writeRegister(0xE6, uReg); 								// Write user register
}

void SI7021::setTempRes(uint8_t res)
{
	uint8_t uReg = readRegister(0xE7);						// Read user register
		if(res == 14)	uReg &= ~(0x81); 					// Set 14-bit temp
		if(res == 13)	uReg = (uReg & ~(0x80)) | (0x80); 	// Set 13-bit temp
		if(res == 12)	uReg = (uReg & ~(0x80)) | (0x01); 	// Set 12-bit temp
		if(res == 11)	uReg |= (0x81); 					// Set 11-bit temp
	writeRegister(0xE6, uReg); 								// Write user register
}

uint8_t SI7021::getHumidityRes()
{
	uint8_t resBits = readRegister(0xE7) & (0x81);			// Read resolution bits
		if(resBits == 0x00)		return 12;					// Return humidity 12-bit
		if(resBits == 0x81)		return 11;					// Return humidity 11-bit
		if(resBits == 0x80)		return 10;					// Return humidity 10-bit
		if(resBits == 0x01)		return  8;					// Return humidity  8-bit
	return 0;												// Return 0 for error
}

uint8_t SI7021::getTempRes()
{
	uint8_t resBits = readRegister(0xE7) & (0x81);			// Read resolution bits
		if(resBits == 0x00)		return 14;					// Return temp 14-bit
		if(resBits == 0x80)		return 13;					// Return temp 13-bit
		if(resBits == 0x01)		return 12;					// Return temp 12-bit
		if(resBits == 0x81)		return 11;					// Return temp 11-bit
	return 0;												// Return 0 for error
}

void SI7021::setHeater(uint8_t hBit)
{
    uint8_t uReg = readRegister(0xE7);						// Read user register
		if(hBit)	uReg |= (0x04);							// Set heater bit - ON
		else		uReg &= ~(0x04);						// Set heater bit - OFF
    writeRegister(0xE6, uReg); 								// Write user register
}

uint8_t SI7021::getHeater()
{
	return ((readRegister(0xE7) & (0x04)) ? 1 : 0);			// Return heater status
}

/*
void SI7021::setHeaterPower(uint8_t hPow)
{
	uint8_t hReg = readRegister(0x11);						// Read heater register
		if(hPow == 1)	hReg &= ~(0x0F);					// Set current 3.09mA
		if(hPow == 2)	hReg = (hReg & ~(0x0F)) | (0x01);	// Set current 9.18mA
		if(hPow == 3)	hReg = (hReg & ~(0x0F)) | (0x02);	// Set current 15.24mA
		if(hPow == 4)	hReg = (hReg & ~(0x0F)) | (0x04);	// Set current 27.39mA
		if(hPow == 5)	hReg = (hReg & ~(0x0F)) | (0x08);	// Set current 51.69mA
		if(hPow == 6)	hReg |= (0x0F);						// Set current 94.20mA
    writeRegister(0x51, hReg);								// Write heater register
}

uint8_t SI7021::getHeaterPower()
{
    uint8_t hReg = readRegister(0x11);						// Read heater register
		if(hReg == 0x00)		return  3;					// Return current 3mA
		if(hReg == 0x01)		return  9;					// Return current 9mA
		if(hReg == 0x02)		return 15;					// Return current 15mA
		if(hReg == 0x04)		return 27;					// Return current 27mA
		if(hReg == 0x08)		return 51;					// Return current 51mA
		if(hReg == 0x0F)		return 94;					// Return current 94mA
	return 0;												// Return 0 for error
}
*/

uint16_t SI7021::getDeviceID()
{
    Wire.beginTransmission(SI7021_ADDR);
	Wire.write(0xFC); 										// Send first address byte
	Wire.write(0xC9); 										// Send second address byte
	Wire.endTransmission(false);
	Wire.requestFrom(SI7021_ADDR, 1); 						// Request first byte
	uint8_t SNB3 = Wire.read(); 							// Read SNB3 byte (ID)
		if(SNB3 == 0x0D)		return 7013;				// Device is Si7013
		else if(SNB3 == 0x14)	return 7020;			 	// Device is Si7020
		else if(SNB3 == 0x15)	return 7021;		 		// Device is Si7021
		else if(SNB3 == 0x00)	return  255; 				// Engineering sample
		else if(SNB3 == 0xFF)	return  255;			 	// Engineering sample
		else					return    0;	 			// Device unknown

}

uint8_t SI7021::getFirmwareVer()
{
    Wire.beginTransmission(SI7021_ADDR);
	Wire.write(0x84); 										// Send first address byte
	Wire.write(0xB8); 										// Send second address byte
	Wire.endTransmission(false);
	Wire.requestFrom(SI7021_ADDR, 1);						// Request first byte
	uint8_t fwV = Wire.read(); 								// Read firmware version
		if(fwV == 0x20)			return 2;					// Firmware version 2.0
		else if(fwV == 0xFF)	return 1; 					// Firmware version 1.0
		else					return 0; 					// Firmware version unknown
}

uint8_t SI7021::checkVDD()
{
	return (readRegister(0xE7) & (0x40)) ? 0 : 1;			// Return 0=LOW / 1=OK
}

void SI7021::reset(uint8_t delayR)
{
	Wire.beginTransmission(SI7021_ADDR);
	Wire.write(0xFE); 										// Write reset command
    Wire.endTransmission();
	delay(delayR);											// Default = 15ms
}

uint16_t SI7021::floatToInt(float FtoI)
{
		return FtoI*100; 									// Converts float to int
}															// keeping full precision

float SI7021::intToFloat(uint16_t ItoF)
{
		return (float)ItoF/100; 							// Convert int to float
}															// keeping full precision

float SI7021::CtoF(float tempC)
{
	return tempC * 9 / 5 + 32; 								// Returns temp in F
}

float SI7021::FtoC(float tempF)
{
	return (tempF - 32) * 5 / 9; 							// Returns temp in C
}

/*  PRIVATE: PRIVATE: PRIVATE: PRIVATE: PRIVATE: PRIVATE: PRIVATE: PRIVATE: */

void SI7021::writeRegister(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(SI7021_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t SI7021::readRegister(uint8_t reg)
{
    Wire.beginTransmission(SI7021_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(SI7021_ADDR, 1);
    return Wire.read();
}

uint16_t SI7021::readSensor(uint8_t reg)
{
    uint16_t var = 0;
    Wire.beginTransmission(SI7021_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    delay(50);
    Wire.requestFrom(SI7021_ADDR, 2);
    var = Wire.read() << 8;
    var |= Wire.read() & 0x0FF;
    return var;
}
