/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.07.2015
Descr: Low-level SDCard functions
*/
#pragma once

#include <SPIBase.h>

/**
 * @brief Intialise SD card interface
 * @param slaveSelect Pin to use for CS
 * @param freqLimit Maximum SPI clock speed
 * @retval bool true on success, false on error
 *
 * It is useful to debug at a lower speed so the logic analyser can catch everything.
 */
bool SDCard_begin(uint8_t slaveSelect, uint32_t freqLimit);

extern SPIBase* SDCardSPI;
