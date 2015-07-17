/*
Author: ADiea
Project: Sming for ESP8266
License: MIT
Date: 15.07.2015
Descr: Implement software SPI for HW configs other than hardware SPI pins(GPIO 12,13,14)
*/
#ifndef _SPI_SOFT_
#define _SPI_SOFT_

#include <SmingCore.h>

class SPISoft
{
public:
	SPISoft(uint16_t miso, uint16_t mosi, uint16_t sck, uint16_t ss):mMISO(miso), mMOSI(mosi), mCLK(sck), mSS(ss){}

	void begin();//setup pins

	void IRAM_ATTR send(const uint8_t* buffer, uint32_t size);

	void IRAM_ATTR recv(uint8_t* buffer, uint32_t size);

	inline void enable(){digitalWrite(mSS, LOW);}

	inline void disable(){digitalWrite(mSS, HIGH);}

	inline void setMOSI(uint8_t val){digitalWrite(mMOSI, val);}

	/**
	\brief Set microsecond delay for the SCK signal. Impacts SPI speed
	*/
	inline void setDelay(uint8_t dly){m_delay = dly;}

private:
	uint16_t mMISO, mMOSI, mCLK, mSS;
	uint8_t m_delay;
};

#endif /*_SPI_SOFT_*/
