/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.07.2015
Descr: Implement software SPI. To improve speed, GPIO16 is not supported(see Digital.cpp)
*/
#include "SPISoft.h"

#define SPEED 0 /* You gain ~0.7 kBps (more for larger data chunks)*/
#define SIZE 1 /* You gain ~ 400B from the total 32K of cache RAM */
#define SPEED_VS_SIZE SIZE /* Your choice here, I choose SIZE */

#define GP_IN(pin)	((GPIO_REG_READ(GPIO_IN_ADDRESS)>>(pin)) & 1)
#define GP_OUT(pin, val) GPIO_REG_WRITE(((((val) != LOW) ? \
							GPIO_OUT_W1TS_ADDRESS : \
							GPIO_OUT_W1TC_ADDRESS)), ((uint16_t)1<<(pin)))
#define SCK_PULSE	GP_OUT(mCLK, HIGH); \
					fastDelay(m_delay); \
					GP_OUT(mCLK, LOW); \
					fastDelay(m_delay);

static inline void IRAM_ATTR fastDelay(unsigned d)
{
	while(d) --d;
}

void SPISoft::begin()
{
	if(16 == mMISO || 16 == mMOSI || 16 == mCLK || 16 == mSS)
	{
		/*To be able to use fast/simple GPIO read/write GPIO16 is not supported*/
		debugf("SPISoft: GPIO 16 not supported\n");
		return;
	}
	
	pinMode(mSS, OUTPUT);
	disable();
	
	pinMode(mCLK, OUTPUT);
	digitalWrite(mCLK, LOW);

	pinMode(mMISO, INPUT);
	digitalWrite(mMISO, HIGH);

	pinMode(mMOSI, OUTPUT);
}

void SPISoft::send(const uint8_t* buffer, uint32_t size)
{
	uint8_t d;
	uint8_t mask; //removed by gcc if not used

	do {
		d = *buffer++;
#if SPEED_VS_SIZE == SIZE
		mask = 0x80;
		do
		{
			GP_OUT(mMOSI, d & mask);
			SCK_PULSE
			mask >>= 1;
		}while(mask != 0);
#else /*SPEED_VS_SIZE != SIZE => loop unroll */
		GP_OUT(mMOSI, d & 0x80);	/* bit7 */
		SCK_PULSE
		GP_OUT(mMOSI, d & 0x40);	/* bit6 */
		SCK_PULSE
		GP_OUT(mMOSI, d & 0x20);	/* bit5 */
		SCK_PULSE
		GP_OUT(mMOSI, d & 0x10);	/* bit4 */
		SCK_PULSE
		GP_OUT(mMOSI, d & 0x08);	/* bit3 */
		SCK_PULSE
		GP_OUT(mMOSI, d & 0x04);	/* bit2 */
		SCK_PULSE
		GP_OUT(mMOSI, d & 0x02);	/* bit1 */
		SCK_PULSE
		GP_OUT(mMOSI, d & 0x01);	/* bit0 */
		SCK_PULSE
#endif
	} while (--size);
}

void SPISoft::recv(uint8_t* buffer, uint32_t size)
{
	uint8_t r;
	uint8_t count;//removed by gcc if not used
	do {
#if SPEED_VS_SIZE == SIZE
		count = 8;
		r = 0;
		do
		{
			r = (r << 1) | GP_IN(mMISO);
			SCK_PULSE
		}while(--count);
#else /*SPEED_VS_SIZE != SIZE => loop unroll*/
		r = 		 GP_IN(mMISO); //bit 7
		SCK_PULSE
		r = r << 1 | GP_IN(mMISO); //bit 6
		SCK_PULSE
		r = r << 1 | GP_IN(mMISO); //bit 5
		SCK_PULSE
		r = r << 1 | GP_IN(mMISO); //bit 4
		SCK_PULSE
		r = r << 1 | GP_IN(mMISO); //bit 3
		SCK_PULSE
		r = r << 1 | GP_IN(mMISO); //bit 2
		SCK_PULSE
		r = r << 1 | GP_IN(mMISO); //bit 1
		SCK_PULSE
		r = r << 1 | GP_IN(mMISO); //bit 0
		SCK_PULSE
#endif
		*buffer++ = r;
	}	while (--size);
}
