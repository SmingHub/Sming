/*
BME280.h - Header file for the BME280 Barometric Pressure Sensor Arduino Library.
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

 Datasheet for BME280:
 http://www.bosch-sensortec.com/content/language1/downloads/BST-BMP180-DS000-07.pdf

*/

#include "BME280.h"
#include "Arduino.h"
//#include <math.h>


 uint8_t osrs_t = 1;             //Temperature oversampling x 1
 uint8_t osrs_p = 1;             //Pressure oversampling x 1
 uint8_t osrs_h = 1;             //Humidity oversampling x 1

 uint8_t t_sb = 4;               //Tstandby, 5=1000ms, 4=500ms
 uint8_t filter = 0;             //Filter off
 uint8_t spi3w_en = 0;           //3-wire SPI Disable
 uint8_t BME280_OperationMode = BME280_MODE_NORMAL;

BME280::BME280()
{
  ConversionWaitTimeMs = 5;
  OversamplingSetting = 0;
  Oversample = false;

  LastTemperatureTime = -1000;
  LastTemperatureData = 0;

  AcceptableTemperatureLatencyForPressure = 1000;

 // SetResolution(BME280_Mode_Standard, false);
}

uint8_t BME280::EnsureConnected()
{
	Read2(Reg_ChipId, 1, buffer);

	if(buffer[0] == ChipIdData)
		IsConnected = 1;
	else
		IsConnected = 0;

	return buffer[0];//IsConnected;
}

void BME280::Initialize()
{
	uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | BME280_OperationMode;
	uint8_t ctrl_hum_reg  = osrs_h;

	uint8_t config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;

	Write(ControlHumidity, ctrl_hum_reg);
	Write(Reg_Control, ctrl_meas_reg);
	Write(Reg_Config, config_reg);


	Read2(Reg_CalibrationTStart, Reg_CalibrationTEnd - Reg_CalibrationTStart + 1, buffer);
	// This data is in Big Endian format from the BME280.
    BME280_REGISTER_DIG_T1 = (buffer[1] << 8) | buffer[0];
    BME280_REGISTER_DIG_T2 = (buffer[3] << 8) | buffer[2];
    BME280_REGISTER_DIG_T3 = (buffer[5] << 8) | buffer[4];

	Read2(Reg_CalibrationPStart, Reg_CalibrationPEnd - Reg_CalibrationPStart + 1, buffer);

    BME280_REGISTER_DIG_P1 = (buffer[1] << 8) | buffer[0];
    BME280_REGISTER_DIG_P2 = (buffer[3] << 8) | buffer[2];
    BME280_REGISTER_DIG_P3 = (buffer[5] << 8) | buffer[4];
    BME280_REGISTER_DIG_P4 = (buffer[7] << 8) | buffer[6];
    BME280_REGISTER_DIG_P5 = (buffer[9] << 8) | buffer[8];
    BME280_REGISTER_DIG_P6 = (buffer[11] << 8) | buffer[10];
    BME280_REGISTER_DIG_P7 = (buffer[13] << 8) | buffer[12];
    BME280_REGISTER_DIG_P8 = (buffer[15] << 8) | buffer[14];
	BME280_REGISTER_DIG_P9 = (buffer[17] << 8) | buffer[16];

	Read2(BME280_REG_DIG_H1, 1, buffer);
	BME280_REGISTER_DIG_H1 = buffer[0];
	Read2(BME280_REG_DIG_H2, 2, buffer);
    BME280_REGISTER_DIG_H2 = (buffer[1] << 8) | buffer[0];
	Read2(BME280_REG_DIG_H3, 1, buffer);
    BME280_REGISTER_DIG_H3 = buffer[0];
    Read2(BME280_REG_DIG_H4, 4, buffer);
    BME280_REGISTER_DIG_H4 = (buffer[0] << 4) | (buffer[1]&0x0F);

    BME280_REGISTER_DIG_H5 = (buffer[2]<<4) | ((buffer[1] & 0xF0)>>4);

	BME280_REGISTER_DIG_H6 = buffer[3];
}

void BME280::PrintCalibrationData()
{
	Serial.print("T1:\t"); Serial.println(BME280_REGISTER_DIG_T1);
	Serial.print("T2:\t"); Serial.println(BME280_REGISTER_DIG_T2);
	Serial.print("T3:\t"); Serial.println(BME280_REGISTER_DIG_T3);

	Serial.print("P1:\t"); Serial.println(BME280_REGISTER_DIG_P1);
	Serial.print("P2:\t"); Serial.println(BME280_REGISTER_DIG_P2);
	Serial.print("P3:\t"); Serial.println(BME280_REGISTER_DIG_P3);
	Serial.print("P4:\t"); Serial.println(BME280_REGISTER_DIG_P4);
	Serial.print("P5:\t"); Serial.println(BME280_REGISTER_DIG_P5);
	Serial.print("P6:\t"); Serial.println(BME280_REGISTER_DIG_P6);
	Serial.print("P7:\t"); Serial.println(BME280_REGISTER_DIG_P7);
	Serial.print("P8:\t"); Serial.println(BME280_REGISTER_DIG_P8);
	Serial.print("P9:\t"); Serial.println(BME280_REGISTER_DIG_P9);

	Serial.print("H1:\t"); Serial.println(BME280_REGISTER_DIG_H1);
	Serial.print("H2:\t"); Serial.println(BME280_REGISTER_DIG_H2);
	Serial.print("H3:\t"); Serial.println(BME280_REGISTER_DIG_H3);
	Serial.print("H4:\t"); Serial.println(BME280_REGISTER_DIG_H4);
	Serial.print("H5:\t"); Serial.println(BME280_REGISTER_DIG_H5);
	Serial.print("H6:\t"); Serial.println(BME280_REGISTER_DIG_H6);

}

int32_t BME280::GetUncompensatedTemperature()
{

	Read2(MeasureTemperature, 3, buffer);
 	int32_t value = ((buffer[0] <<12) | (buffer[1] <<4) | (buffer[2] >>4));

    return value;
}

int32_t BME280::GetUncompensatedPressure()
{

		Read2(MeasurePressure, 3, buffer);

		int32_t value = ((buffer[0] <<12) | (buffer[1] <<4) | (buffer[2] >>4));

    return value ;
}

int32_t BME280::GetUncompensatedHumidity()
{

		Read2(MeasureHumidity, 2, buffer);

		int32_t value = ((buffer[0] <<8) | buffer[1]);

    return value ;
}


float BME280::CompensateTemperature(int32_t uncompensatedTemperature)
{

	//long temperature;
	int32_t var1,var2;
	var1 = ((((uncompensatedTemperature>>3) - ((int32_t)BME280_REGISTER_DIG_T1<<1))) * ((int32_t)BME280_REGISTER_DIG_T2)) >> 11;
	var2 = (((((uncompensatedTemperature>>4) - ((int32_t)BME280_REGISTER_DIG_T1)) * ((uncompensatedTemperature>>4) - ((int32_t)BME280_REGISTER_DIG_T1))) >> 12) * ((int32_t)BME280_REGISTER_DIG_T3)) >> 14;
	t_fine = var1 + var2;
	float temperature = (t_fine * 5 + 128) >> 8;  /* temperature in 0.01 deg C*/

	return temperature/100;


}
//
float BME280::CompensatePressure(int32_t uncompensatedPressure) {

	GetTemperature();
	int64_t var1, var2, p;

	  var1 = ((int64_t)t_fine) - 128000;
	  var2 = var1 * var1 * (int64_t)BME280_REGISTER_DIG_P6;
	  var2 = var2 + ((var1*(int64_t)BME280_REGISTER_DIG_P5)<<17);
	  var2 = var2 + (((int64_t)BME280_REGISTER_DIG_P4)<<35);
	  var1 = ((var1 * var1 * (int64_t)BME280_REGISTER_DIG_P3)>>8) + ((var1 * (int64_t)BME280_REGISTER_DIG_P2)<<12);
	  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)BME280_REGISTER_DIG_P1)>>33;

	  if (var1 == 0) {
		return 0;  // avoid exception caused by division by zero
	  }
	  p = 1048576 - uncompensatedPressure;
	  p = (((p<<31) - var2)*3125) / var1;
	  var1 = (((int64_t)BME280_REGISTER_DIG_P9) * (p>>13) * (p>>13)) >> 25;
	  var2 = (((int64_t)BME280_REGISTER_DIG_P8) * p) >> 19;

	  p = ((p + var1 + var2) >> 8) + (((int64_t)BME280_REGISTER_DIG_P7)<<4);
	  return (float)p/256;

}

float BME280::CompensateHumidity(int32_t uncompensatedPressure) {

	GetTemperature();


	  int32_t v_x1_u32r;

	  v_x1_u32r = (t_fine - ((int32_t)76800));

	  v_x1_u32r = (((((uncompensatedPressure << 14) - (((int32_t)BME280_REGISTER_DIG_H4) << 20) -
			  (((int32_t)BME280_REGISTER_DIG_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
		       (((((((v_x1_u32r * ((int32_t)BME280_REGISTER_DIG_H6)) >> 10) *
			    (((v_x1_u32r * ((int32_t)BME280_REGISTER_DIG_H3)) >> 11) + ((int32_t)32768))) >> 10) +
			  ((int32_t)2097152)) * ((int32_t)BME280_REGISTER_DIG_H2) + 8192) >> 14));

	  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
				     ((int32_t)BME280_REGISTER_DIG_H1)) >> 4));

	  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
	  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
	  float h = (v_x1_u32r>>12);
	  return  h / 1024.0;

}

void BME280::SoftReset()
{
    Write(Reg_SoftReset, SoftResetInstruction);
    delay(100);
}

float BME280::GetTemperature()
{
    return CompensateTemperature(GetUncompensatedTemperature());
}

float BME280::GetPressure()
{
    return CompensatePressure(GetUncompensatedPressure());
}

float BME280::GetHumidity()
{
    return CompensateHumidity(GetUncompensatedHumidity());
}

/*float BME280::GetAltitude(float currentSeaLevelPressureInPa)
{
    // Get pressure in Pascals (Pa).
    float pressure = GetPressure();
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - powf(pressure / currentSeaLevelPressureInPa, (float)0.1902949571836346));
    return altitude;
}*/

//uint8_t BME280::SetResolution(uint8_t sampleResolution, bool oversample)
//{
//    OversamplingSetting = sampleResolution;
//    Oversample = oversample;
//    switch (sampleResolution)
//    {
//        case 0:
//            ConversionWaitTimeMs = 5;
//            break;
//        case 1:
//            ConversionWaitTimeMs = 8;
//            break;
//        case 2:
//            ConversionWaitTimeMs = 14;
//            break;
//        case 3:
//            ConversionWaitTimeMs = 26;
//            break;
//        default:
//            return ErrorCode_1_Num;
//    }
//}

void BME280::Write(int address, int data)
{
  Wire.beginTransmission(BME280_Address);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

void BME280::Read2(int address, int length, uint8_t buffer[])
{
  Wire.beginTransmission(BME280_Address);
  Wire.write(address);
  Wire.endTransmission();

  Wire.beginTransmission(BME280_Address);
  Wire.requestFrom(BME280_Address, length);

  while(Wire.available())
  {
	  for(uint8_t i = 0; i < length; i++)
	  {
		  buffer[i] = Wire.read();
	  }
  }
  Wire.endTransmission();
}

const char* BME280::GetErrorText(int errorCode)
{
	if(ErrorCode_1_Num == 1)
		return (const char*)ErrorCode_1;

	return (const char*)"Error not defined.";
}
