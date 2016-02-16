#ifndef __SI7021_H__
#define __SI7021_H__

#include "Arduino.h"

class SI7021
{
	public:
			SI7021(int pinSCL, int pinSDA);
			/* Main Functions */
			void begin();
			bool isPresent();
			float readHumidity();
			float readTemp();
			float readTempPrev();

			/* Resolution Functions */
			void setHumidityRes(uint8_t res);
			void setTempRes(uint8_t res);
			uint8_t getHumidityRes();
			uint8_t getTempRes();

			/* Heater Functions */
			void setHeater(uint8_t hBit);
			uint8_t getHeater();
			//void setHeaterPower(uint8_t hPow); // broken : register access problem
			//uint8_t getHeaterPower(); // broken : register access problem

			/* Device ID And Reset Functions */
			uint16_t getDeviceID();
			uint8_t getFirmwareVer();
			uint8_t checkVDD();
			void reset(uint8_t delayR = 15);

			/* Conversion Functions */
			uint16_t floatToInt(float FtoI);
			float intToFloat(uint16_t ItoF);
			float CtoF(float tempC);
			float FtoC(float tempF);

		private:
			int SCL;
			int SDA;
			bool present;
			void writeRegister(uint8_t reg, uint8_t value);
			uint8_t readRegister(uint8_t reg);
			uint16_t readSensor(uint8_t reg);

};

#endif
