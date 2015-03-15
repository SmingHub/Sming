#include "BH1750FVI.h"
#include "Arduino.h"

BH1750FVI::BH1750FVI(byte address)
{
	address_value = address;
}

bool BH1750FVI::begin(void)
{
	Wire.begin();
	return I2CWriteTo(BH1750_Power_On); //Turn it On
}

void BH1750FVI::sleep(void)
{
	I2CWriteTo(BH1750_Power_Down); //Turn it off , Reset operator won't work in this mode
}

void BH1750FVI::reset(void)
{
	I2CWriteTo(BH1750_Power_On); //Turn it on again
	I2CWriteTo(BH1750_Reset); //Reset
}

void BH1750FVI::setMode(uint8_t MODE)
{
	switch (MODE)
	{
	case BH1750_Continuous_H_resolution_Mode:
		break;
	case BH1750_Continuous_H_resolution_Mode2:
		break;
	case BH1750_Continuous_L_resolution_Mode:
		break;
	case BH1750_OneTime_H_resolution_Mode:
		break;
	case BH1750_OneTime_H_resolution_Mode2:
		break;
	case BH1750_OneTime_L_resolution_Mode:
		break;
	}
	delay(10);
	I2CWriteTo(MODE);
}

uint16_t BH1750FVI::getLightIntensity(void)
{
	uint16_t Intensity_value;
	Wire.beginTransmission(address_value);
	int res = Wire.requestFrom(address_value, 2);
	Intensity_value = Wire.read();
	Intensity_value <<= 8;
	Intensity_value |= Wire.read();
	Wire.endTransmission();
	if (res == 0) return 0;

	Intensity_value = Intensity_value / 1.2;
	return Intensity_value;
}

bool BH1750FVI::I2CWriteTo(uint8_t DataToSend)
{
	Wire.beginTransmission(address_value);
	Wire.write(DataToSend);
	return Wire.endTransmission();
}
