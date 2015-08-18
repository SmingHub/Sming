/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.07.2015
Descr: Low-level SDCard functions
*/
#ifndef _SD_CARD_
#define _SD_CARD_

#include <SmingCore.h>

void SDCard_begin();

extern SPISoft *SDCardSPI;

#endif /*_SD_CARD_*/
