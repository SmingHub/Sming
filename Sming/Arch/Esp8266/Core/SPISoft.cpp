/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.07.2015
Descr: Implement software SPI. To improve speed, GPIO16 is not supported(see Digital.cpp)
*/
#include "SPISoft.h"
#include <esp_systemapi.h>

#define SPEED 0			   /* You gain ~0.7 kBps (more for larger data chunks)*/
#define SIZE 1			   /* You gain ~ 400B from the total 32K of cache RAM */
#define SPEED_VS_SIZE SIZE /* Your choice here, I choose SIZE */

#define GP_IN(pin) ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> (pin)) & 1)
#define GP_OUT(pin, val) GPIO_REG_WRITE(((val) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS), BIT(pin))

#define SCK_SETUP() GP_OUT(mCLK, !cksample)
#define SCK_SAMPLE() GP_OUT(mCLK, cksample)
#define SCK_IDLE() GP_OUT(mCLK, cpol)
#define MOSI_WRITE(d) GP_OUT(mMOSI, d)
#define MISO_READ() GP_IN(mMISO)

static __forceinline void fastDelay(int d)
{
	while(d > 0) {
		__asm__ volatile("nop");
		--d;
	}
}

bool SPISoft::begin()
{
	if(16 == mMISO || 16 == mMOSI || 16 == mCLK) {
		/*To be able to use fast/simple GPIO read/write GPIO16 is not supported*/
		debugf("SPISoft: GPIO 16 not supported\n");
		return false;
	}

	pinMode(mCLK, OUTPUT);

	pinMode(mMISO, INPUT);
	digitalWrite(mMISO, HIGH);

	pinMode(mMOSI, OUTPUT);

	prepare(SPIDefaultSettings);

	return true;
}

uint32_t SPISoft::transferWordLSB(uint32_t word, uint8_t bits)
{
	uint32_t res{0};
	word <<= 32 - bits;
	for(uint32_t mask = BIT(32 - bits); mask != 0; mask <<= 1) {
		uint8_t d = (word & mask) ? 1 : 0;

		if(d != cksample) {
			GPIO_REG_WRITE((d ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS), BIT(mCLK) | BIT(mMOSI));
		} else {
			SCK_SETUP();
			MOSI_WRITE(d);
		}
		fastDelay(m_delay);
		res >>= 1;
		SCK_SAMPLE();
		res |= MISO_READ() << 31;
		fastDelay(m_delay);
	}

	res >>= 32 - bits;
	return res;
}

uint32_t SPISoft::transferWordMSB(uint32_t word, uint8_t bits)
{
	uint32_t res{0};
	for(uint32_t mask = BIT(bits - 1); mask != 0; mask >>= 1) {
		uint8_t d = (word & mask) ? 1 : 0;

		if(d != cksample) {
			GPIO_REG_WRITE((d ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS), BIT(mCLK) | BIT(mMOSI));
		} else {
			SCK_SETUP();
			MOSI_WRITE(d);
		}
		fastDelay(m_delay);
		res <<= 1;
		SCK_SAMPLE();
		res |= GP_IN(mMISO);
		fastDelay(m_delay);
	}

	return res;
}

uint32_t SPISoft::transfer32(uint32_t val, uint8_t bits)
{
	if(lsbFirst) {
		val = transferWordLSB(val, bits);
	} else {
		val = transferWordMSB(val, bits);
	}

	return val;
}

void SPISoft::transfer(uint8_t* buffer, uint32_t size)
{
	if(lsbFirst) {
		while(size-- != 0) {
			*buffer = transferWordLSB(*buffer, 8);
			++buffer;
		}
	} else {
		while(size-- != 0) {
			*buffer = transferWordMSB(*buffer, 8);
			++buffer;
		}
	}
}

void SPISoft::prepare(SPISettings& settings)
{
	dataMode = settings.dataMode;
	auto mode_num = SPISettings::getModeNum(dataMode);
	cpol = (mode_num & 0x02) >> 1;
	uint8_t cpha = mode_num & 0x01;
	cksample = (cpol == cpha);

	lsbFirst = (settings.bitOrder != MSBFIRST);
	SCK_SETUP();
}

void SPISoft::endTransaction()
{
	SCK_IDLE();
}
