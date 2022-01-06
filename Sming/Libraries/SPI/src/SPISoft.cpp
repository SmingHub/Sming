/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPISoft.cpp
 *
 * Author: (github.com/)ADiea
 * Project: Sming for ESP8266 - https://github.com/anakod/Sming
 * License: MIT
 * Date: 15.07.2015
 * Descr: Implement software SPI. To improve speed, GPIO16 is not supported(see Digital.cpp)
 *
 * @author mikee47 January 2022
 *
 ****/

#include "SPISoft.h"
#include <esp_systemapi.h>
#include <spisoft_arch.h>

#ifndef SPISOFT_ARCH_DELAY_FIXED
#define SPISOFT_ARCH_DELAY_FIXED 0
#endif

#define FUNC_OPT __attribute__((optimize(3)))

namespace spisoft
{
#define SCK_SETUP() GP_OUT(pins.sck, ~cksample)
#define SCK_SAMPLE() GP_OUT(pins.sck, cksample)
#define SCK_IDLE() GP_OUT(pins.sck, cpol)
#define MOSI_WRITE(d) GP_OUT(pins.mosi, d)
#define MISO_READ() GP_IN(pins.miso)

// Enabling variable delays disables fixed delays
#ifdef SPISOFT_DELAY_VARIABLE
#undef SPISOFT_DELAY_FIXED
#define SPISOFT_DELAY_FIXED 0
#endif

static_assert(SPISOFT_DELAY_FIXED >= 0 && SPISOFT_DELAY_FIXED <= 20, "SPISOFT_DELAY_FIXED invalid");

template <int n = SPISOFT_DELAY_FIXED + SPISOFT_ARCH_DELAY_FIXED> __forceinline void fixedDelay()
{
	__asm__ volatile("nop" :::);
	fixedDelay<n - 1>();
}

template <> void fixedDelay<0>()
{
}

__forceinline void fastDelay(int d)
{
	fixedDelay();
#ifdef SPISOFT_DELAY_VARIABLE
	while(d-- > 0) {
		__asm__ volatile("nop" :::);
	}
#else
	(void)d;
#endif
}

} // namespace spisoft

using namespace spisoft;

SPISoft::SPISoft() : SPIBase(spisoft::defaultPins)
{
}

SPISoft::SPISoft(uint8_t delay) : SPIBase(spisoft::defaultPins), m_delay(delay)
{
}

bool SPISoft::begin()
{
#ifdef ARCH_ESP8266
	if(16 == pins.miso || 16 == pins.mosi || 16 == pins.sck) {
		/*To be able to use fast/simple GPIO read/write GPIO16 is not supported*/
		debugf("SPISoft: GPIO 16 not supported\n");
		return false;
	}
#endif

	if(pins.sck == SPI_PIN_DEFAULT) {
		mPins.sck = defaultPins.sck;
	}
	pinMode(pins.sck, OUTPUT);

	if(pins.miso == SPI_PIN_DEFAULT) {
		mPins.miso = defaultPins.miso;
	}
	pinMode(pins.miso, INPUT);
	digitalWrite(pins.miso, HIGH);

	if(pins.mosi == SPI_PIN_DEFAULT) {
		mPins.mosi = defaultPins.mosi;
	}
	pinMode(pins.mosi, OUTPUT);

	prepare(SPIDefaultSettings);

	return true;
}

// Write/read a bit in LSB-first order
#define CLOCK_LSB(bit, shift)                                                                                          \
	SCK_SETUP();                                                                                                       \
	MOSI_WRITE(word >> bit);                                                                                           \
	fastDelay(m_delay);                                                                                                \
	res >>= 1;                                                                                                         \
	SCK_SAMPLE();                                                                                                      \
	res |= MISO_READ() << shift;                                                                                       \
	fastDelay(m_delay - 7);

#define CLOCK_LSB8(bit) CLOCK_LSB(bit, 7)

uint8_t FUNC_OPT SPISoft::transferByteLSB(uint8_t word)
{
	uint8_t res{0};
	CLOCK_LSB8(0)
	CLOCK_LSB8(1)
	CLOCK_LSB8(2)
	CLOCK_LSB8(3)
	CLOCK_LSB8(4)
	CLOCK_LSB8(5)
	CLOCK_LSB8(6)
	CLOCK_LSB8(7)
	return res;
}

uint32_t FUNC_OPT SPISoft::transferWordLSB(uint32_t word, uint8_t bits)
{
	uint32_t res{0};

	switch(bits / 8) {
	case 4:
		res = transferByteLSB(word) << 24;
		word >>= 8;
	case 3:
		res = (res >> 8) | transferByteLSB(word) << 24;
		word >>= 8;
	case 2:
		res = (res >> 8) | transferByteLSB(word) << 24;
		word >>= 8;
	case 1:
		res = (res >> 8) | transferByteLSB(word) << 24;
		word >>= 8;
	}

	for(uint8_t bit = 0; bit < (bits % 8); ++bit) {
		CLOCK_LSB(bit, 31);
	}

	res >>= 32 - bits;
	return res;
}

// Write/read a bit in MSB-first order
#define CLOCK_MSB32(bit)                                                                                               \
	SCK_SETUP();                                                                                                       \
	MOSI_WRITE(word >> bit);                                                                                           \
	fastDelay(m_delay);                                                                                                \
	res <<= 1;                                                                                                         \
	SCK_SAMPLE();                                                                                                      \
	res |= MISO_READ();                                                                                                \
	fastDelay(m_delay - 7);

uint8_t FUNC_OPT SPISoft::transferByteMSB(uint8_t word)
{
	uint8_t res{0};
	CLOCK_MSB32(7)
	CLOCK_MSB32(6)
	CLOCK_MSB32(5)
	CLOCK_MSB32(4)
	CLOCK_MSB32(3)
	CLOCK_MSB32(2)
	CLOCK_MSB32(1)
	CLOCK_MSB32(0)
	return res;
}

uint32_t FUNC_OPT SPISoft::transferWordMSB(uint32_t word, uint8_t bits)
{
	uint32_t res{0};
	uint8_t bit = bits;
	while((bit & 7) != 0) {
		--bit;
		CLOCK_MSB32(bit);
	}

	switch(bit / 8) {
	case 4:
		res = transferByteMSB(word >> 24);
	case 3:
		res = (res << 8) | transferByteMSB(word >> 16);
	case 2:
		res = (res << 8) | transferByteMSB(word >> 8);
	case 1:
		res = (res << 8) | transferByteMSB(word);
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

void SPISoft::transfer(uint8_t* buffer, size_t size)
{
	if(lsbFirst) {
		while(size-- != 0) {
			*buffer = transferByteLSB(*buffer);
			++buffer;
		}
	} else {
		while(size-- != 0) {
			*buffer = transferByteMSB(*buffer);
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

#if defined(SPISOFT_DELAY_VARIABLE) && defined(ARCH_ESP8266)
	// If user doesn't specify speed then don't override
	if(settings.speed.frequency != 0) {
		m_delay = checkSpeed(settings.speed);
	}
#endif
}

void SPISoft::endTransaction()
{
	SCK_IDLE();
}
