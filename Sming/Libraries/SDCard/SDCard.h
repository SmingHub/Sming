/*
Author: ADiea
Project: Sming for ESP8266
License: MIT
Date: 16.07.2015
Descr: low level SDCard functions
*/
#ifndef _SD_CARD_
#define _SD_CARD_

#include <SmingCore.h>

class SDCardClass
{
public:
	SDCardClass(uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t ss) :
			mSPI(miso, mosi, sck, ss){}

	void begin();

	void send(const uint8_t* buffer, uint32_t size);

	void recv(uint8_t* buffer, uint32_t size);

	inline void enable(){mSPI.enable();}

	inline void disable(){mSPI.disable();}

	inline void setSPIDelay(uint8_t dly){mSPI.setDelay(dly);}

	//inline SPISoft* getSPI(){return &mSPI;}
private:
	SPISoft mSPI;
	FATFS mFatFs;		/* FatFs work area needed for each volume */
};

extern SDCardClass SDCard;

#endif /*_SD_CARD_*/
