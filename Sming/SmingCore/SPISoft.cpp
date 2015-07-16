/*

Author: ADiea
Project: Sming for ESP8266
License: MIT
Date: 15.07.2015
Descr: Implement software SPI for HW configs other than hardware SPI pins(GPIO 12,13,14). 
		For speed considerations, GPIO16 is not supported(see Digital.cpp)
*/
#include "SPISoft.h"

#define GP_IN(pin)			((GPIO_REG_READ(GPIO_IN_ADDRESS)>>(pin)) & 1)

#define GP_OUT(pin, val) 	GPIO_REG_WRITE((((val != LOW) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS)), (1<<(pin)))

#define SCK_PULSE	GP_OUT(mCLK, HIGH); \
					delayMicroseconds(m_usDelay); \
					GP_OUT(mCLK, LOW); \
					delayMicroseconds(m_usDelay);

void SPISoft::begin()
{
	if(16 == mMISO ||
		16 == mMOSI ||
		16 == mCLK ||
		16 == mSS)
	{
		debugf("SPISoft: GPIO 16 not supported\n"); /*...by this lib. Please choose a different pin*/
	}
	
	pinMode(mMISO, INPUT);
	digitalWrite(mMISO, HIGH);

	pinMode(mMOSI, OUTPUT);
	
	pinMode(mCLK, OUTPUT);
	digitalWrite(mCLK, LOW);

	pinMode(mSS, OUTPUT);
	disable();
}

void SPISoft::send(uint8_t d)
{
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
}

uint8_t SPISoft::recv()
{
	uint8_t r=0;
	
	r = 		  GP_IN(mMISO); //bit 7
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
}
