/*
BMP180.h - Header file for the BMP180 Barometric Pressure Sensor Arduino Library.
Copyright (C) 2012 Love Electronics Ltd (loveelectronics.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Datasheet for BMP180:
 http://www.bosch-sensortec.com/content/language1/downloads/BST-BMP180-DS000-07.pdf

*/

#include "BMP180.h"
#include "Arduino.h"
//#include <math.h>

BMP180::BMP180()
{
  ConversionWaitTimeMs = 5;
  OversamplingSetting = 0;
  Oversample = false;

  LastTemperatureTime = -1000;
  LastTemperatureData = 0;

  AcceptableTemperatureLatencyForPressure = 1000;

  SetResolution(BMP180_Mode_Standard, false);
}

uint8_t BMP180::EnsureConnected()
{
	Read2(Reg_ChipId, 1, buffer);

	if(buffer[0] == ChipIdData)
		IsConnected = 1;
	else
		IsConnected = 0;

	return IsConnected;
}

void BMP180::Initialize()
{
	Read2(Reg_CalibrationStart, Reg_CalibrationEnd - Reg_CalibrationStart + 2, buffer);
	// This data is in Big Endian format from the BMP180.
    Calibration_AC1 = (buffer[0] << 8) | buffer[1];
    Calibration_AC2 = (buffer[2] << 8) | buffer[3];
    Calibration_AC3 = (buffer[4] << 8) | buffer[5];
    Calibration_AC4 = (buffer[6] << 8) | buffer[7];
    Calibration_AC5 = (buffer[8] << 8) | buffer[9];
    Calibration_AC6 = (buffer[10] << 8) | buffer[11];
    Calibration_B1 = (buffer[12] << 8) | buffer[13];
    Calibration_B2 = (buffer[14] << 8) | buffer[15];
    Calibration_MB = (buffer[16] << 8) | buffer[17];
    Calibration_MC = (buffer[18] << 8) | buffer[19];
    Calibration_MD = (buffer[20] << 8) | buffer[21];
}

void BMP180::PrintCalibrationData()
{
	Serial.print("AC1:\t"); Serial.println(Calibration_AC1);
	Serial.print("AC2:\t"); Serial.println(Calibration_AC2);
	Serial.print("AC3:\t"); Serial.println(Calibration_AC3);
	Serial.print("AC4:\t"); Serial.println(Calibration_AC4);
	Serial.print("AC5:\t"); Serial.println(Calibration_AC5);
	Serial.print("AC6:\t"); Serial.println(Calibration_AC6);
	Serial.print("B1:\t"); Serial.println(Calibration_B1);
	Serial.print("B2:\t"); Serial.println(Calibration_B2);
	Serial.print("MB:\t"); Serial.println(Calibration_MB);
	Serial.print("MC:\t"); Serial.println(Calibration_MC);
	Serial.print("MD:\t"); Serial.println(Calibration_MD);
}

int BMP180::GetUncompensatedTemperature()
{
    // Instruct device to perform a conversion.
    Write(Reg_Control, ControlInstruction_MeasureTemperature);
    // Wait for the conversion to complete.
    delay(5);
    Read2(Reg_AnalogConverterOutMSB, 2, buffer);
    int value = (buffer[0] << 8) | buffer[1];
    return value;
}

long BMP180::GetUncompensatedPressure()
{
    long pressure = 0;
    int loops = Oversample ? 3 : 1;

    for (int i = 0; i < loops; i++)
    {
        // Instruct device to perform a conversion, including the oversampling data.
        uint8_t CtrlByte = ControlInstruction_MeasurePressure + (OversamplingSetting << 6);
        Write(Reg_Control, CtrlByte);
        // Wait for the conversion
        delay(ConversionWaitTimeMs);
        // Read the conversion data.
        uint8_t buffer[3];
		Read2(Reg_AnalogConverterOutMSB, 3, buffer);

        // Collect the data (and push back the LSB if we are not sampling them).
        pressure = ((((long)buffer[0] <<16) | ((long)buffer[1] <<8) | ((long)buffer[2])) >> (8-OversamplingSetting));
    }
    return pressure / loops;
}

float BMP180::CompensateTemperature(int uncompensatedTemperature)
{
    int temperature;
    int x2;
	long x1;
	x1 = (((long)uncompensatedTemperature - (long)Calibration_AC6) * (long)Calibration_AC5) >> 15;
    x2 = ((long)Calibration_MC << 11) / (x1 + Calibration_MD);
    int param_b5 = x1 + x2;
    temperature = (int)((param_b5 + 8) >> 4);  /* temperature in 0.1 deg C*/
    float fTemperature = temperature;
	fTemperature /= 10.0;

    // Record this data because it is required by the pressure algorithem.
    LastTemperatureData = param_b5;
    LastTemperatureTime = millis();

    return fTemperature;
}

long BMP180::CompensatePressure(long uncompensatedPressure)
{
	int msSinceLastTempReading = millis() - LastTemperatureTime;
    // Check to see if we have old temperature data.
    if (msSinceLastTempReading > AcceptableTemperatureLatencyForPressure)
        GetTemperature(); // Refresh the temperature.

    // Data from the BMP180 datasheet to test algorithm.
    /*OversamplingSetting = 0;
    uncompensatedPressure = 23843;
    LastTemperatureData = 2399;
    Calibration_AC1 = 408;
    Calibration_AC2 = -72;
    Calibration_AC3 = -14383;
    Calibration_AC4 = 32741;
    Calibration_AC5 = 32757;
    Calibration_AC6 = 23153;
    Calibration_B1 = 6190;
    Calibration_B2 = 4;
    Calibration_MB = -32767;
    Calibration_MC = -8711;
    Calibration_MD = 2868;*/

    // Algorithm taken from BMP180 datasheet.
    long b6 = LastTemperatureData - 4000;
    long x1 = (Calibration_B2 * (b6 * b6 >> 12)) >> 11;
    long x2 = Calibration_AC2 * b6 >> 11;
    long x3 = x1 + x2;
    long b3 = ((Calibration_AC1 * 4 + x3) << OversamplingSetting) + 2;
    b3 = b3 >> 2;
    x1 = Calibration_AC3 * b6 >> 13;
    x2 = (Calibration_B1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    long b4 = Calibration_AC4 * (x3 + 32768) >> 15;
    unsigned long b7 = (((uncompensatedPressure - b3)) * (50000 >> OversamplingSetting));
    long p;
    if (b7 < 0x80000000)
	{
		p = ((b7 * 2) / b4);    
	}
    else
	{
        p = ((b7 / b4) * 2);
	}
		
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);

    return p;
}

void BMP180::SoftReset()
{
    Write(Reg_SoftReset, SoftResetInstruction);
    delay(100);
}

float BMP180::GetTemperature()
{
    return CompensateTemperature(GetUncompensatedTemperature());
}

long BMP180::GetPressure()
{
    return CompensatePressure(GetUncompensatedPressure());
}

/*float BMP180::GetAltitude(float currentSeaLevelPressureInPa)
{
    // Get pressure in Pascals (Pa).
    float pressure = GetPressure();
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - powf(pressure / currentSeaLevelPressureInPa, (float)0.1902949571836346));
    return altitude;
}*/

uint8_t BMP180::SetResolution(uint8_t sampleResolution, bool oversample)
{
    OversamplingSetting = sampleResolution;
    Oversample = oversample;
    switch (sampleResolution)
    {
        case 0:
            ConversionWaitTimeMs = 5;
            break;
        case 1:
            ConversionWaitTimeMs = 8;
            break;
        case 2:
            ConversionWaitTimeMs = 14;
            break;
        case 3:
            ConversionWaitTimeMs = 26;
            break;
        default:
            return ErrorCode_1_Num;
    }

    return 0;
}

void BMP180::Write(int address, int data)
{
  Wire.beginTransmission(BMP180_Address);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

void BMP180::Read2(int address, int length, uint8_t buffer[])
{
  Wire.beginTransmission(BMP180_Address);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.beginTransmission(BMP180_Address);
  Wire.requestFrom(BMP180_Address, length);

  while(Wire.available())
  {
	  for(uint8_t i = 0; i < length; i++)
	  {
		  buffer[i] = Wire.read();
	  }
  }
  Wire.endTransmission();
}

const char* BMP180::GetErrorText(int errorCode)
{
	if(ErrorCode_1_Num == 1)
		return (const char*)ErrorCode_1;
	
	return (const char*)"Error not defined.";
}
