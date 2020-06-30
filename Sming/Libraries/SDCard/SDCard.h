/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.07.2015
Descr: Low-level SDCard functions
*/
#pragma once

#include <SmingCore.h>
#include "SPISoft.h"

void SDCard_begin(uint8 PIN_CARD_SS, uint8 byteOrder, uint32 freqLimit);

extern SPIBase* SDCardSPI;
